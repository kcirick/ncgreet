#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "Globals.hh"
#include "Authenticator.hh"

//------------------------------------------------------------------------
Response::Response (Json::Value json_resp) {
   if(!json_resp["type"])  return;

   string type = json_resp["type"].asString();
   if(type=="success")           mType = Success;
   else if(type=="error")        mType = Error;
   else if(type=="auth_message") mType = AuthMessage;
   else                          mType = Invalid;

   if(mType==Error) {
      string errortype = json_resp["error_type"].asString();
      if(errortype == "auth_error") mErrorType = ErrorTypeAuth;
      else                          mErrorType = ErrorTypeGeneric;

      mErrorMsg = mErrorType==ErrorTypeGeneric ? "Generic error" : "Authentication error";
   }

   if(mType==AuthMessage) {
      string authtype = json_resp["auth_message_type"].asString();
      if(authtype == "visible")     mAuthType = AuthTypeVisible;
      else if(authtype == "secret") mAuthType = AuthTypeSecret;
      else if(authtype == "info")   mAuthType = AuthTypeInfo;
      else                          mAuthType = AuthTypeError;
      
      mAuthMsg = json_resp["auth_message"].asString();
   }
}

//------------------------------------------------------------------------
Response
Authenticator::handle_request(enum HandleType handle_type, string value) {
   Json::Value request;
   Json::Value cmd;

   switch(handle_type) {
      case CREATE_SESSION:
         request[ "type" ] = "create_session";
         request[ "username" ] = value;
         break;
      case START_SESSION:
         cmd[0] = value;
         request[ "type" ] = "start_session";
         request[ "cmd" ] = cmd;
         break;
      case POST_AUTH:
         request[ "type" ] = "post_auth_message_response";
         request[ "response" ] = value;
         break;
      case CANCEL_SESSION:
         request[ "type" ] = "cancel_session";
         break;
   } 

   Json::Value response;
   response = json_roundtrip(request);

   return Response(response);
}

Json::Value
Authenticator::json_roundtrip(Json::Value input) {
   Json::Value output = Json::objectValue;

   string greetd_sock = getenv("GREETD_SOCK");
   if(greetd_sock.empty()) {
      cerr << "GREETD_SOCK is not set!" << endl;
      close(mFD);
   }

   //--- connect
   mFD = socket(AF_UNIX, SOCK_STREAM, 0);
   if(mFD<0) {
      cerr << "Unable to open socket" << endl;
      return false;
   }

   struct sockaddr_un addr;
   memset(&addr, 0, sizeof(addr));
   addr.sun_family = AF_UNIX;
   strcpy(addr.sun_path, greetd_sock.c_str());
   if(connect(mFD, (struct sockaddr*)&addr, sizeof(addr)) == -1){
      cerr << "Unable to connect to socket" << endl;
      return output;
   }

   //--- write
   Json::FastWriter writer;
   string input_str = writer.write(input);
   uint32_t wriLen = input_str.size();
   char * chrWriLen = (char*)&wriLen;

   // chrlen will be 4 bytes long
   if(write(mFD, chrWriLen, 4) != 4) {
      cerr << "Error sending message" << endl;
      return output;
   }

   if(write(mFD, input_str.c_str(), wriLen) != wriLen) {
      cerr << "Error sending message" << endl;
      return output;
   }

   //--- read
   uint32_t retLen;
   char * chrRetLen = (char*)&retLen;

   if(read(mFD, chrRetLen, 4) != 4) {
      cerr << "Error reading response" << endl;
      return output;
   }

   char message[ retLen+1 ] = { '\0' };
   memset(message, '\0', retLen+1);
   if(read(mFD, message, retLen) != retLen) {
      cerr << "Error reading response" << endl;
      return output;
   }

   //--- success
   Json::Reader reader;
   reader.parse(message, output);

   close(mFD);
   return output;
}

bool 
Authenticator::authenticate(const char* uname, const char* passwd) {
   mErrorMsg.clear();

   Response resp = handle_request(CREATE_SESSION, uname);

   if(resp.getResponseType() == Response::Invalid) {
      mErrorMsg = "Unable to communicate with greetd.";
      return false;
   }

   // Request successful: This may happen for passwordless login
   if(resp.getResponseType() == Response::Success){
      return true;
   }

   // Request failed: Cancel the session
   if(resp.getResponseType() == Response::Error) {
      mErrorMsg = resp.getErrorMsg();
      resp = handle_request(CANCEL_SESSION, "");

      return false;
   }

   // Request authentication:
   if(resp.getResponseType() == Response::AuthMessage) {
      switch(resp.getAuthType()) {
         case Response::AuthTypeInvalid:
            mErrorMsg = "Not an auth_message response. Abort.";
            return false;
            break;
         case Response::AuthTypeVisible:
         case Response::AuthTypeSecret:
            resp = handle_request(POST_AUTH, passwd);

            // Authentication successful!
            if(resp.getResponseType() == Response::Success){
               return true;
            }

            mErrorMsg = "Authentication failed.";
            resp = handle_request(CANCEL_SESSION, "");

            return false;
            break;
         case Response::AuthTypeInfo:
         case Response::AuthTypeError:
            mErrorMsg = resp.getAuthMsg();
            resp = handle_request(CANCEL_SESSION, "");

            return false;
            break;
      }
   }
   // We shouldn't get here
   return false;
}

bool
Authenticator::start_session(const char* session_cmd){

   Response resp = handle_request(START_SESSION, session_cmd);

   // Success
   if( resp.getResponseType() == Response::Success) {
      return true;
   }

   mErrorMsg = resp.getErrorMsg();
   resp = handle_request(CANCEL_SESSION, "");

   return false;
}
