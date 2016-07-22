#ifndef N_MDUSER_H_
#define N_MDUSER_H_

#include "stdafx.h"
#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <fstream>
#include <node.h>
#include "ThostFtdcTraderApi.h"
#include "ThostFtdcUserApiDataType.h"
#include <uv.h>
#include "uv_mduser.h"

#include "node_object_wrap.h"
#include <v8-util.h>

using namespace v8;

extern bool islog;
extern void logger_cout(const char* content);
extern std::string to_string(int val);

class WrapMdUser :public node::ObjectWrap {
public:
    WrapMdUser(void);
    ~WrapMdUser(void);

    static void Connect(const FunctionCallbackInfo<Value>& args);
    static void On(const FunctionCallbackInfo<Value>& args);
    static void ReqUserLogin(const FunctionCallbackInfo<Value>& args);
    static void ReqUserLogout(const FunctionCallbackInfo<Value>& args);
    static void  SubscribeMarketData(const FunctionCallbackInfo<Value>& args);
    static void  UnSubscribeMarketData(const FunctionCallbackInfo<Value>& args);
    static void Disposed(const FunctionCallbackInfo<Value>& args);
    static void Init(Isolate*,int args);
    static void NewInstance(const FunctionCallbackInfo<Value>& args);

private:
    static void initEventMap();
    static void New(const FunctionCallbackInfo<Value>& args);
    static void pkg_cb_userlogin(CbRtnField* data, Local<Value>*cbArray);
    static void pkg_cb_userlogout(CbRtnField* data, Local<Value>*cbArray);
    static void pkg_cb_rspsubmarketdata(CbRtnField* data, Local<Value>*cbArray);
    static void pkg_cb_unrspsubmarketdata(CbRtnField* data, Local<Value>*cbArray);
    static void pkg_cb_rtndepthmarketdata(CbRtnField* data, Local<Value>*cbArray); 
    static void pkg_cb_rsperror(CbRtnField* data, Local<Value>*cbArray);
    static Local<Value> pkg_rspinfo(CThostFtdcRspInfoField *pRspInfo);

    uv_mduser* uvMdUser;
    static int s_uuid;
    static Isolate* isolate;
    static void FunCallback(CbRtnField *data);
    static void FunRtnCallback(int result, void* baton);
    static Persistent<Function> constructor;
    static std::map<const char*, int,ptrCmp> event_map;

    //using local static instead
    static PersistentValueMap<int, Function ,DefaultPersistentValueMapTraits<int,Function> >&  callback_map(Isolate*);
    static PersistentValueMap<int, Function ,DefaultPersistentValueMapTraits<int,Function> >&  fun_rtncb_map(Isolate*);     
};


#endif
