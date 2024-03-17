#ifndef AUTHENTICATOR_HH
#define AUTHENTICATOR_HH

#include <json/json.h>

class Response {
   public:
      enum Type {
         Invalid     = 0,     /* Response was invalid */
         Success     = 1,     /* Request was success */
         Error       = 2,     /* Request failed */
         AuthMessage = 3,     /* Request succeeed, auth required */
      };

      enum AuthType {
         AuthTypeInvalid   = 0,     /* Not an Authentication response */
         AuthTypeVisible   = 1,     /* Authentication in visible form */
         AuthTypeSecret    = 2,     /* Authentication in secret form */
         AuthTypeInfo      = 3,     /* Authentication type info */
         AuthTypeError     = 4,     /* Authentication failed */
      };

      enum ErrorType {
         ErrorTypeNone     = 0,     /* No error */
         ErrorTypeGeneric  = 1,     /* Generic error */
         ErrorTypeAuth     = 2,     /* Authentication error */
      };

      Response(Json::Value);
      
      Response::Type getResponseType() { return mType; };
      Response::AuthType getAuthType() { return mAuthType; };
      Response::ErrorType getErrorType() { return mErrorType; };

      string getAuthMsg() { return mAuthMsg; };
      string getErrorMsg() { return mErrorMsg; };

   private:
      Response::Type mType = Invalid;
      Response::AuthType mAuthType = AuthTypeInvalid;
      Response::ErrorType mErrorType = ErrorTypeNone;

      string mAuthMsg;
      string mErrorMsg;
};

class Authenticator {
   public:
      Authenticator() { };
      ~Authenticator() { };

      bool authenticate(const char*, const char*);
      bool start_session(const char*);

      string getErrorMsg() { return mErrorMsg; };

   private:
      enum HandleType {
         CREATE_SESSION    = 0,
         START_SESSION     = 1,
         POST_AUTH         = 2,
         CANCEL_SESSION    = 3,
      };

      Json::Value json_roundtrip(Json::Value);
      Response handle_request(enum HandleType, string);

      int mFD = -1;        // GreetD socket FD

      string mErrorMsg;    // Error message
};
#endif
