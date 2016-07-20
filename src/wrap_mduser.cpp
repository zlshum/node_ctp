#include <node.h>
#include "wrap_mduser.h"

Persistent<Function> WrapMdUser::constructor;
int WrapMdUser::s_uuid;
std::map<const char*, int,ptrCmp> WrapMdUser::event_map;
std::map<int, Persistent<Function> > WrapMdUser::callback_map;
std::map<int, Persistent<Function> > WrapMdUser::fun_rtncb_map;

Isolate* WrapMdUser::isolate = NULL;

WrapMdUser::WrapMdUser() {
    logger_cout("wrap_mduser------>object start init");
    uvMdUser = new uv_mduser();
    logger_cout("wrap_mduser------>object init successed");
}

WrapMdUser::~WrapMdUser() {
    if(uvMdUser){
        delete uvMdUser;
    }
    logger_cout("wrape_mduser------>object destroyed");
}

void WrapMdUser::Init(Isolate* iso,int args) {
    // Prepare constructor template
    isolate = iso;
    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate,New);
    tpl->SetClassName(String::NewFromUtf8(isolate,"WrapMdUser"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    // Prototype
    tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate,"on"),
        FunctionTemplate::New(isolate,On)->GetFunction());

    tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate,"connect"),
        FunctionTemplate::New(isolate,Connect)->GetFunction());

    tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate,"reqUserLogin"),
        FunctionTemplate::New(isolate,ReqUserLogin)->GetFunction());

    tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate,"reqUserLogout"),
        FunctionTemplate::New(isolate,ReqUserLogout)->GetFunction());

    tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate,"subscribeMarketData"),
        FunctionTemplate::New(isolate,SubscribeMarketData)->GetFunction());

    tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate,"unSubscribeMarketData"),
        FunctionTemplate::New(isolate,UnSubscribeMarketData)->GetFunction());    

    tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate,"disconnect"),
        FunctionTemplate::New(isolate,Disposed)->GetFunction());

    constructor.Reset(isolate,tpl->GetFunction());
}

void WrapMdUser::initEventMap() {
    event_map["connect"] = T_ON_CONNECT;
    event_map["disconnected"] = T_ON_DISCONNECTED;
    event_map["rspUserLogin"] = T_ON_RSPUSERLOGIN;
    event_map["rspUserLogout"] = T_ON_RSPUSERLOGOUT;
    event_map["rspSubMarketData"] = T_ON_RSPSUBMARKETDATA;
    event_map["rspUnSubMarketData"] = T_ON_RSPUNSUBMARKETDATA;
    event_map["rtnDepthMarketData"] = T_ON_RTNDEPTHMARKETDATA;
    event_map["rspError"] = T_ON_RSPERROR;
}

void WrapMdUser::New(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();

    if (event_map.size() == 0)
        initEventMap();
    WrapMdUser* obj = new WrapMdUser();
    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
}

void WrapMdUser::NewInstance(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();

    const unsigned argc = 1;
    Local<Value> argv[argc] = { args[0] };
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    Local<Context> context = isolate->GetCurrentContext();
    Local<Object> instance = cons->NewInstance(context, argc, argv).ToLocalChecked();
    args.GetReturnValue().Set(instance);
}

void WrapMdUser::On(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    if (args[0]->IsUndefined() || args[1]->IsUndefined()) {
        logger_cout("Wrong FunctionCallbackInfo<Value>->event name or function");
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>->event name or function")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }

    WrapMdUser* obj = ObjectWrap::Unwrap<WrapMdUser>(args.This());

    Local<String> eventName = args[0]->ToString();
    Local<Function> cb = Local<Function>::Cast(args[1]);
    Persistent<Function> unRecoveryCb = Persistent<Function>::New(cb);
    String::AsciiValue eNameAscii(eventName);

    std::map<const char*, int>::iterator eIt = event_map.find(*eNameAscii);
    if (eIt == event_map.end()) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"System has no register this event")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    std::map<int, Persistent<Function> >::iterator cIt = callback_map.find(eIt->second);
    if (cIt != callback_map.end()) {
        logger_cout("Callback is defined before");
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Callback is defined before")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }

    callback_map[eIt->second] = unRecoveryCb;
    obj->uvMdUser->On(*eNameAscii,eIt->second, FunCallback);
    return scope.Close(Int32::New(isolate,0));
}

void WrapMdUser::Connect(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    std::string log = "wrap_mduser Connect------>";
    if (args[0]->IsUndefined()) {
        logger_cout("Wrong FunctionCallbackInfo<Value>->front addr");
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>->front addr")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }    
    int uuid = -1;
    WrapMdUser* obj = ObjectWrap::Unwrap<WrapMdUser>(args.This());
    if (!args[2]->IsUndefined() && args[2]->IsFunction()) {
        uuid = ++s_uuid;
        fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[2]));
        std::string _head = std::string(log);
        logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
    }

    Local<String> frontAddr = args[0]->ToString();
    Local<String> szPath = args[1]->IsUndefined() ? String::NewFromUtf8(isolate,"m") : args[0]->ToString();
    String::AsciiValue addrAscii(frontAddr);
    String::AsciiValue pathAscii(szPath);

    UVConnectField pConnectField;
    memset(&pConnectField, 0, sizeof(pConnectField));
    strcpy(pConnectField.front_addr, ((std::string)*addrAscii).c_str());
    strcpy(pConnectField.szPath, ((std::string)*pathAscii).c_str());  
    logger_cout(log.append(" ").append((std::string)*addrAscii).append("|").append((std::string)*pathAscii).append("|").c_str());
    obj->uvMdUser->Connect(&pConnectField, FunRtnCallback, uuid);
            args.GetReturnValue().Set(Undefined(isolate));
        return;
}

void WrapMdUser::ReqUserLogin(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    std::string log = "wrap_mduser ReqUserLogin------>";
    if (args[0]->IsUndefined() || args[1]->IsUndefined() || args[2]->IsUndefined()) {
        std::string _head = std::string(log);
        logger_cout(_head.append(" Wrong FunctionCallbackInfo<Value>").c_str());
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>")));
                args.GetReturnValue().Set(Undefined(isolate));
        return;
    }

    int uuid = -1;
    WrapMdUser* obj = ObjectWrap::Unwrap<WrapMdUser>(args.This());
    if (!args[3]->IsUndefined() && args[3]->IsFunction()) {
        uuid = ++s_uuid;
        fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[3]));
        std::string _head = std::string(log);
        logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
    }

    Local<String> broker = args[0]->ToString();
    Local<String> userId = args[1]->ToString();
    Local<String> pwd = args[2]->ToString();
    String::AsciiValue brokerAscii(broker);
    String::AsciiValue userIdAscii(userId);
    String::AsciiValue pwdAscii(pwd);

    CThostFtdcReqUserLoginField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, ((std::string)*brokerAscii).c_str());
    strcpy(req.UserID, ((std::string)*userIdAscii).c_str());
    strcpy(req.Password, ((std::string)*pwdAscii).c_str());
    logger_cout(log.append(" ").append((std::string)*brokerAscii).append("|").append((std::string)*userIdAscii).append("|").append((std::string)*pwdAscii).c_str());
    obj->uvMdUser->ReqUserLogin(&req, FunRtnCallback, uuid);
    args.GetReturnValue().Set(Undefined(isolate));
    return;
}

void WrapMdUser::ReqUserLogout(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    std::string log = "wrap_mduser ReqUserLogout------>";

    if (args[0]->IsUndefined() || args[1]->IsUndefined()) {
        std::string _head = std::string(log);
        logger_cout(_head.append(" Wrong FunctionCallbackInfo<Value>").c_str());
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    int uuid = -1;
    WrapMdUser* obj = ObjectWrap::Unwrap<WrapMdUser>(args.This());
    if (!args[2]->IsUndefined() && args[2]->IsFunction()) {
        uuid = ++s_uuid;
        fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[2]));
        std::string _head = std::string(log);
        logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
    }

    Local<String> broker = args[0]->ToString();
    Local<String> userId = args[1]->ToString();
    String::AsciiValue brokerAscii(broker);
    String::AsciiValue userIdAscii(userId);

    CThostFtdcUserLogoutField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, ((std::string)*brokerAscii).c_str());
    strcpy(req.UserID, ((std::string)*userIdAscii).c_str());
    logger_cout(log.append(" ").append((std::string)*brokerAscii).append("|").append((std::string)*userIdAscii).c_str());
    obj->uvMdUser->ReqUserLogout(&req, FunRtnCallback, uuid);
    args.GetReturnValue().Set(Undefined(isolate));
    return;
}

void WrapMdUser::SubscribeMarketData(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    std::string log = "wrap_mduser SubscribeMarketData------>";

    if (args[0]->IsUndefined() || !args[0]->IsArray()) {
        std::string _head = std::string(log);
        logger_cout(_head.append(" Wrong FunctionCallbackInfo<Value>").c_str());
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    int uuid = -1;
    WrapMdUser* obj = ObjectWrap::Unwrap<WrapMdUser>(args.This());
    if (!args[1]->IsUndefined() && args[1]->IsFunction()) {
        uuid = ++s_uuid;
        fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[1]));
        std::string _head = std::string(log);
        logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
    } 
    Local<v8::Array> instrumentIDs = Local<v8::Array>::Cast(args[0]);
    char** idArray = new char*[instrumentIDs->Length()];
    
    for (uint32_t i = 0; i < instrumentIDs->Length(); i++) {
        Local<String> instrumentId = instrumentIDs->Get(i)->ToString();
        String::AsciiValue idAscii(instrumentId);           
        char* id = new char[instrumentId->Length() + 1];
        strcpy(id, *idAscii);
        idArray[i] = id;
        log.append(*idAscii).append("|");
    }
    logger_cout(log.c_str());
    obj->uvMdUser->SubscribeMarketData(idArray, instrumentIDs->Length(), FunRtnCallback, uuid);
    delete idArray;
    args.GetReturnValue().Set(Undefined(isolate));
    return;
}

void WrapMdUser::UnSubscribeMarketData(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    std::string log = "wrap_mduser UnSubscribeMarketData------>";

    if (args[0]->IsUndefined() || !args[0]->IsArray()) {
        std::string _head = std::string(log);
        logger_cout(_head.append(" Wrong FunctionCallbackInfo<Value>").c_str());
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    int uuid = -1;
    WrapMdUser* obj = ObjectWrap::Unwrap<WrapMdUser>(args.This());
    if (!args[1]->IsUndefined() && args[1]->IsFunction()) {
        uuid = ++s_uuid;
        fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[1]));
        std::string _head = std::string(log);
        logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
    }
    Local<v8::Array> instrumentIDs = Local<v8::Array>::Cast(args[0]);
    char** idArray = new char*[instrumentIDs->Length()];

    for (uint32_t i = 0; i < instrumentIDs->Length(); i++) {
        Local<String> instrumentId = instrumentIDs->Get(i)->ToString();
        String::AsciiValue idAscii(instrumentId);
        char* id = new char[instrumentId->Length() + 1];
        strcpy(id, *idAscii);
        idArray[i] = id;
        log.append(*idAscii).append("|");
    }
    logger_cout(log.c_str());
    obj->uvMdUser->UnSubscribeMarketData(idArray, instrumentIDs->Length(), FunRtnCallback, uuid);
            args.GetReturnValue().Set(Undefined(isolate));
        return;     
}

void WrapMdUser::Disposed(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    WrapMdUser* obj = ObjectWrap::Unwrap<WrapMdUser>(args.This());
    obj->uvMdUser->Disposed();
    std::map<int, Persistent<Function> >::iterator callback_it = callback_map.begin();
    while (callback_it != callback_map.end()) {
        callback_it->second.Dispose();
        callback_it++;
    }
    event_map.clear();
    callback_map.clear();
    fun_rtncb_map.clear();
    delete obj->uvMdUser;
    obj->uvMdUser = NULL;
    logger_cout("wrap_mduser Disposed------>wrap disposed");
    args.GetReturnValue().Set(Undefined(isolate));
    return;
}


////////////////////////////////////////////////////////////////////////////////////////////////

void WrapMdUser::FunCallback(CbRtnField *data) {
    Isolate* isolate = args.GetIsolate();
    std::map<int, Persistent<Function> >::iterator cIt = callback_map.find(data->eFlag);
    if (cIt == callback_map.end())
        return;

    switch (data->eFlag) {
    case T_ON_CONNECT:
    {
                         Local<Value> argv[1] = { Local<Value>::New(Undefined(isolate)) };
                         cIt->second->Call(Context::GetCurrent()->Global(), 1, argv);
                         break;
    }
    case T_ON_DISCONNECTED:
    {
                              Local<Value> argv[1] = { Int32::New(isolate,data->nReason) };
                              cIt->second->Call(Context::GetCurrent()->Global(), 1, argv);
                              break;
    }
    case T_ON_RSPUSERLOGIN:
    {
                              Local<Value> argv[4];
                              pkg_cb_userlogin(data, argv);
                              cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);
                              break;
    }
    case T_ON_RSPUSERLOGOUT:
    {
                               Local<Value> argv[4];
                               pkg_cb_userlogout(data, argv);
                               cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);
                               break;
    }
    case T_ON_RSPSUBMARKETDATA:
    {
                                  Local<Value> argv[4];
                                  pkg_cb_rspsubmarketdata(data, argv);
                                  cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);
                                  break;
    }
    case T_ON_RSPUNSUBMARKETDATA:
    {
                                    Local<Value> argv[4];
                                    pkg_cb_unrspsubmarketdata(data, argv);
                                    cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);
                                    break;
    }
    case T_ON_RTNDEPTHMARKETDATA:
    {
                                    Local<Value> argv[1];
                                    pkg_cb_rtndepthmarketdata(data, argv);
                                    cIt->second->Call(Context::GetCurrent()->Global(), 1, argv);
                                    break;
    }
    case T_ON_RSPERROR:
    {
                          Local<Value> argv[3];
                          pkg_cb_rsperror(data, argv);
                          cIt->second->Call(Context::GetCurrent()->Global(), 3, argv);

                          break;
    }
    }
    
}
void WrapMdUser::FunRtnCallback(int result, void* baton) {
    Isolate* isolate = args.GetIsolate();
    LookupCtpApiBaton* tmp = static_cast<LookupCtpApiBaton*>(baton);
    if (tmp->uuid != -1) {
        std::map<const int, Persistent<Function> >::iterator it = fun_rtncb_map.find(tmp->uuid);
        Local<Value> argv[1] = { Local<Value>::New(Int32::New(isolate,tmp->nResult)) };
        it->second->Call(Context::GetCurrent()->Global(), 1, argv);
        it->second.Dispose();
        fun_rtncb_map.erase(tmp->uuid);
    }
    
}
void WrapMdUser::pkg_cb_userlogin(CbRtnField* data, Local<Value>*cbArray) {
    *cbArray = Int32::New(isolate,data->nRequestID);
    *(cbArray + 1) = Boolean::New(isolate,data->bIsLast)->ToBoolean();
    CThostFtdcRspUserLoginField* pRspUserLogin = static_cast<CThostFtdcRspUserLoginField*>(data->rtnField);
    CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField*>(data->rspInfo);
    if (pRspUserLogin) {
        Local<Object> jsonRtn = Object::New();
        jsonRtn->Set(String::NewFromUtf8(isolate,"TradingDay"), String::NewFromUtf8(isolate,pRspUserLogin->TradingDay));
        jsonRtn->Set(String::NewFromUtf8(isolate,"LoginTime"), String::NewFromUtf8(isolate,pRspUserLogin->LoginTime));
        jsonRtn->Set(String::NewFromUtf8(isolate,"BrokerID"), String::NewFromUtf8(isolate,pRspUserLogin->BrokerID));
        jsonRtn->Set(String::NewFromUtf8(isolate,"UserID"), String::NewFromUtf8(isolate,pRspUserLogin->UserID));
        jsonRtn->Set(String::NewFromUtf8(isolate,"SystemName"), String::NewFromUtf8(isolate,pRspUserLogin->SystemName));
        jsonRtn->Set(String::NewFromUtf8(isolate,"FrontID"), Int32::New(isolate,pRspUserLogin->FrontID));
        jsonRtn->Set(String::NewFromUtf8(isolate,"SessionID"), Int32::New(isolate,pRspUserLogin->SessionID));
        jsonRtn->Set(String::NewFromUtf8(isolate,"MaxOrderRef"), String::NewFromUtf8(isolate,pRspUserLogin->MaxOrderRef));
        jsonRtn->Set(String::NewFromUtf8(isolate,"SHFETime"), String::NewFromUtf8(isolate,pRspUserLogin->SHFETime));
        jsonRtn->Set(String::NewFromUtf8(isolate,"DCETime"), String::NewFromUtf8(isolate,pRspUserLogin->DCETime));
        jsonRtn->Set(String::NewFromUtf8(isolate,"CZCETime"), String::NewFromUtf8(isolate,pRspUserLogin->CZCETime));
        jsonRtn->Set(String::NewFromUtf8(isolate,"FFEXTime"), String::NewFromUtf8(isolate,pRspUserLogin->FFEXTime));
        jsonRtn->Set(String::NewFromUtf8(isolate,"INETime"), String::NewFromUtf8(isolate,pRspUserLogin->INETime));
        *(cbArray + 2) = jsonRtn;
    }
    else {
        *(cbArray + 2) = Local<Value>::New(Undefined(isolate));
    }

    *(cbArray + 3) = pkg_rspinfo(pRspInfo);
    return;
}

void WrapMdUser::pkg_cb_userlogout(CbRtnField* data, Local<Value>*cbArray) {
    *cbArray = Int32::New(isolate,data->nRequestID);
    *(cbArray + 1) = Boolean::New(isolate,data->bIsLast)->ToBoolean();
    CThostFtdcRspUserLoginField* pRspUserLogin = static_cast<CThostFtdcRspUserLoginField*>(data->rtnField);
    CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField*>(data->rspInfo);
    if (pRspUserLogin) {
        Local<Object> jsonRtn = Object::New();
        jsonRtn->Set(String::NewFromUtf8(isolate,"BrokerID"), String::NewFromUtf8(isolate,pRspUserLogin->BrokerID));
        jsonRtn->Set(String::NewFromUtf8(isolate,"UserID"), String::NewFromUtf8(isolate,pRspUserLogin->UserID));
        *(cbArray + 2) = jsonRtn;
    }
    else {
        *(cbArray + 2) = Local<Value>::New(Undefined(isolate));
    }
    *(cbArray + 3) = pkg_rspinfo(pRspInfo);
    return;
}
void WrapMdUser::pkg_cb_rspsubmarketdata(CbRtnField* data, Local<Value>*cbArray) {
    *cbArray = Int32::New(isolate,data->nRequestID);
    *(cbArray + 1) = Boolean::New(isolate,data->bIsLast)->ToBoolean();
    CThostFtdcSpecificInstrumentField *pSpecificInstrument = static_cast<CThostFtdcSpecificInstrumentField*>(data->rtnField);
    CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField*>(data->rspInfo);
    if (pSpecificInstrument) {
        Local<Object> jsonRtn = Object::New();
        jsonRtn->Set(String::NewFromUtf8(isolate,"InstrumentID"), String::NewFromUtf8(isolate,pSpecificInstrument->InstrumentID));
        *(cbArray + 2) = jsonRtn;
    }
    else {
        *(cbArray + 2) = Local<Value>::New(Undefined(isolate));
    }
    *(cbArray + 3) = pkg_rspinfo(pRspInfo);
    return;
}
void WrapMdUser::pkg_cb_unrspsubmarketdata(CbRtnField* data, Local<Value>*cbArray) {
    *cbArray = Int32::New(isolate,data->nRequestID);
    *(cbArray + 1) = Boolean::New(isolate,data->bIsLast)->ToBoolean();
    CThostFtdcSpecificInstrumentField *pSpecificInstrument = static_cast<CThostFtdcSpecificInstrumentField*>(data->rtnField);
    CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField*>(data->rspInfo);
    if (pSpecificInstrument) {
        Local<Object> jsonRtn = Object::New();
        jsonRtn->Set(String::NewFromUtf8(isolate,"InstrumentID"), String::NewFromUtf8(isolate,pSpecificInstrument->InstrumentID));
        *(cbArray + 2) = jsonRtn;
    }
    else {
        *(cbArray + 2) = Local<Value>::New(Undefined(isolate));
    }
    *(cbArray + 3) = pkg_rspinfo(pRspInfo);
    return;
}
void WrapMdUser::pkg_cb_rtndepthmarketdata(CbRtnField* data, Local<Value>*cbArray) {
    CThostFtdcDepthMarketDataField *pDepthMarketData = static_cast<CThostFtdcDepthMarketDataField*>(data->rtnField);
    if (pDepthMarketData) {               
        Local<Object> jsonRtn = Object::New();
        jsonRtn->Set(String::NewFromUtf8(isolate,"TradingDay"), String::NewFromUtf8(isolate,pDepthMarketData->TradingDay));
        jsonRtn->Set(String::NewFromUtf8(isolate,"InstrumentID"), String::NewFromUtf8(isolate,pDepthMarketData->InstrumentID));
        jsonRtn->Set(String::NewFromUtf8(isolate,"ExchangeID"), String::NewFromUtf8(isolate,pDepthMarketData->ExchangeID));
        jsonRtn->Set(String::NewFromUtf8(isolate,"ExchangeInstID"), String::NewFromUtf8(isolate,pDepthMarketData->ExchangeInstID));
        jsonRtn->Set(String::NewFromUtf8(isolate,"LastPrice"), Number::New(isolate,pDepthMarketData->LastPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate,"PreSettlementPrice"), Number::New(isolate,pDepthMarketData->PreSettlementPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate,"PreClosePrice"), Number::New(isolate,pDepthMarketData->PreClosePrice));
        jsonRtn->Set(String::NewFromUtf8(isolate,"PreOpenInterest"), Number::New(isolate,pDepthMarketData->PreOpenInterest));
        jsonRtn->Set(String::NewFromUtf8(isolate,"OpenPrice"), Number::New(isolate,pDepthMarketData->OpenPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate,"HighestPrice"), Number::New(isolate,pDepthMarketData->HighestPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate,"LowestPrice"), Number::New(isolate,pDepthMarketData->LowestPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate,"Volume"), Int32::New(isolate,pDepthMarketData->Volume));
        jsonRtn->Set(String::NewFromUtf8(isolate,"Turnover"), Number::New(isolate,pDepthMarketData->Turnover));
        jsonRtn->Set(String::NewFromUtf8(isolate,"OpenInterest"), Number::New(isolate,pDepthMarketData->OpenInterest));
        jsonRtn->Set(String::NewFromUtf8(isolate,"ClosePrice"), Number::New(isolate,pDepthMarketData->ClosePrice));
        jsonRtn->Set(String::NewFromUtf8(isolate,"SettlementPrice"), Number::New(isolate,pDepthMarketData->SettlementPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate,"UpperLimitPrice"), Number::New(isolate,pDepthMarketData->UpperLimitPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate,"LowerLimitPrice"), Number::New(isolate,pDepthMarketData->LowerLimitPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate,"PreDelta"), Number::New(isolate,pDepthMarketData->PreDelta));
        jsonRtn->Set(String::NewFromUtf8(isolate,"CurrDelta"), Number::New(isolate,pDepthMarketData->CurrDelta));
        jsonRtn->Set(String::NewFromUtf8(isolate,"UpdateTime"), String::NewFromUtf8(isolate,pDepthMarketData->UpdateTime));
        jsonRtn->Set(String::NewFromUtf8(isolate,"UpdateMillisec"), Int32::New(isolate,pDepthMarketData->UpdateMillisec));
        jsonRtn->Set(String::NewFromUtf8(isolate,"BidPrice1"), Number::New(isolate,pDepthMarketData->BidPrice1));
        jsonRtn->Set(String::NewFromUtf8(isolate,"BidVolume1"), Number::New(isolate,pDepthMarketData->BidVolume1));
        jsonRtn->Set(String::NewFromUtf8(isolate,"AskPrice1"), Number::New(isolate,pDepthMarketData->AskPrice1));
        jsonRtn->Set(String::NewFromUtf8(isolate,"AskVolume1"), Number::New(isolate,pDepthMarketData->AskVolume1));
        jsonRtn->Set(String::NewFromUtf8(isolate,"BidPrice2"), Number::New(isolate,pDepthMarketData->BidPrice2));
        jsonRtn->Set(String::NewFromUtf8(isolate,"BidVolume2"), Number::New(isolate,pDepthMarketData->BidVolume2));
        jsonRtn->Set(String::NewFromUtf8(isolate,"AskPrice2"), Number::New(isolate,pDepthMarketData->AskPrice2));
        jsonRtn->Set(String::NewFromUtf8(isolate,"AskVolume2"), Number::New(isolate,pDepthMarketData->AskVolume2));
        jsonRtn->Set(String::NewFromUtf8(isolate,"BidPrice3"), Number::New(isolate,pDepthMarketData->BidPrice3));
        jsonRtn->Set(String::NewFromUtf8(isolate,"BidVolume3"), Number::New(isolate,pDepthMarketData->BidVolume3));
        jsonRtn->Set(String::NewFromUtf8(isolate,"AskPrice3"), Number::New(isolate,pDepthMarketData->AskPrice3));
        jsonRtn->Set(String::NewFromUtf8(isolate,"AskVolume3"), Number::New(isolate,pDepthMarketData->AskVolume3));
        jsonRtn->Set(String::NewFromUtf8(isolate,"BidPrice4"), Number::New(isolate,pDepthMarketData->BidPrice4));
        jsonRtn->Set(String::NewFromUtf8(isolate,"BidVolume4"), Number::New(isolate,pDepthMarketData->BidVolume4));
        jsonRtn->Set(String::NewFromUtf8(isolate,"AskPrice4"), Number::New(isolate,pDepthMarketData->AskPrice4));
        jsonRtn->Set(String::NewFromUtf8(isolate,"AskVolume4"), Number::New(isolate,pDepthMarketData->AskVolume4));
        jsonRtn->Set(String::NewFromUtf8(isolate,"BidPrice5"), Number::New(isolate,pDepthMarketData->BidPrice5));
        jsonRtn->Set(String::NewFromUtf8(isolate,"BidVolume5"), Number::New(isolate,pDepthMarketData->BidVolume5));
        jsonRtn->Set(String::NewFromUtf8(isolate,"AskPrice5"), Number::New(isolate,pDepthMarketData->AskPrice5));
        jsonRtn->Set(String::NewFromUtf8(isolate,"AskVolume5"), Number::New(isolate,pDepthMarketData->AskVolume5));
        jsonRtn->Set(String::NewFromUtf8(isolate,"AveragePrice"), Number::New(isolate,pDepthMarketData->AveragePrice));
        jsonRtn->Set(String::NewFromUtf8(isolate,"ActionDay"), String::NewFromUtf8(isolate,pDepthMarketData->ActionDay));           
        *cbArray = jsonRtn;
    }
    else {
        *cbArray = Local<Value>::New(Undefined(isolate));
    }
    
    return;
}  
void WrapMdUser::pkg_cb_rsperror(CbRtnField* data, Local<Value>*cbArray) {
    *cbArray = Int32::New(isolate,data->nRequestID);
    *(cbArray + 1) = Boolean::New(isolate,data->bIsLast)->ToBoolean();
    CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField*>(data->rspInfo);
    *(cbArray + 2) = pkg_rspinfo(pRspInfo);
    return;
}

Local<Value> WrapMdUser::pkg_rspinfo(CThostFtdcRspInfoField *pRspInfo) {
    if (pRspInfo) {
        Local<Object> jsonInfo = Object::New();
        jsonInfo->Set(String::NewFromUtf8(isolate,"ErrorID"), Int32::New(isolate,pRspInfo->ErrorID));
        jsonInfo->Set(String::NewFromUtf8(isolate,"ErrorMsg"), String::NewFromUtf8(isolate,pRspInfo->ErrorMsg));
        return jsonInfo;
    }
    else {
        return Local<Value>::New(Undefined(isolate));
    }
}
