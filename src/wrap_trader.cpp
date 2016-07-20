#include <node.h>
#include "wrap_trader.h"

Persistent<Function> WrapTrader::constructor;
int WrapTrader::s_uuid;
std::map<const char*, int,ptrCmp> WrapTrader::event_map;
std::map<int, Persistent<Function> > WrapTrader::callback_map;
std::map<int, Persistent<Function> > WrapTrader::fun_rtncb_map;

WrapTrader::WrapTrader() {    
    logger_cout("wrap_trader------>object start init");
    uvTrader = new uv_trader();    
    logger_cout("wrap_trader------>object init successed");
}

WrapTrader::~WrapTrader(void) {
    if(uvTrader){
        delete uvTrader;
    }
    logger_cout("wrap_trader------>object destroyed");
}

void WrapTrader::Init(int args) {
    // Prepare constructor template
    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate,New);
    tpl->SetClassName(String::NewFromUtf8(isolate,"WrapTrader"));
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

    tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate,"reqSettlementInfoConfirm"),
    FunctionTemplate::New(isolate,ReqSettlementInfoConfirm)->GetFunction());

    tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate,"reqQryInstrument"),
    FunctionTemplate::New(isolate,ReqQryInstrument)->GetFunction());

    tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate,"reqQryTradingAccount"),
    FunctionTemplate::New(isolate,ReqQryTradingAccount)->GetFunction());

    tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate,"reqQryInvestorPosition"),
    FunctionTemplate::New(isolate,ReqQryInvestorPosition)->GetFunction());

    tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate,"reqQryInvestorPositionDetail"),
    FunctionTemplate::New(isolate,ReqQryInvestorPositionDetail)->GetFunction());

    tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate,"reqOrderInsert"),
    FunctionTemplate::New(isolate,ReqOrderInsert)->GetFunction());

    tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate,"reqOrderAction"),
    FunctionTemplate::New(isolate,ReqOrderAction)->GetFunction());

    tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate,"reqQryInstrumentMarginRate"),
    FunctionTemplate::New(isolate,ReqQryInstrumentMarginRate)->GetFunction());

    tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate,"reqQryDepthMarketData"),
    FunctionTemplate::New(isolate,ReqQryDepthMarketData)->GetFunction());

    tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate,"reqQrySettlementInfo"),
    FunctionTemplate::New(isolate,ReqQrySettlementInfo)->GetFunction());

    tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate,"disconnect"),
    FunctionTemplate::New(isolate,Disposed)->GetFunction());

    tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate,"getTradingDay"),
    FunctionTemplate::New(isolate,GetTradingDay)->GetFunction());

    constructor = Persistent<Function>::New(tpl->GetFunction());
}

void WrapTrader::initEventMap() {
    event_map["connect"] = T_ON_CONNECT;
    event_map["disconnected"] = T_ON_DISCONNECTED;
    event_map["rspUserLogin"] = T_ON_RSPUSERLOGIN;
    event_map["rspUserLogout"] = T_ON_RSPUSERLOGOUT;
    event_map["rspInfoconfirm"] = T_ON_RSPINFOCONFIRM;
    event_map["rspInsert"] = T_ON_RSPINSERT;
    event_map["errInsert"] = T_ON_ERRINSERT;
    event_map["rspAction"] = T_ON_RSPACTION;
    event_map["errAction"] = T_ON_ERRACTION;
    event_map["rqOrder"] = T_ON_RQORDER;
    event_map["rtnOrder"] = T_ON_RTNORDER;
    event_map["rqTrade"] = T_ON_RQTRADE;
    event_map["rtnTrade"] = T_ON_RTNTRADE;
    event_map["rqInvestorPosition"] = T_ON_RQINVESTORPOSITION;
    event_map["rqInvestorPositionDetail"] = T_ON_RQINVESTORPOSITIONDETAIL;
    event_map["rqTradingAccount"] = T_ON_RQTRADINGACCOUNT;
    event_map["rqInstrument"] = T_ON_RQINSTRUMENT;
    event_map["rqDdpthmarketData"] = T_ON_RQDEPTHMARKETDATA;
    event_map["rqSettlementInfo"] = T_ON_RQSETTLEMENTINFO;
    event_map["rspError"] = T_ON_RSPERROR;
}

void WrapTrader::New(const FunctionCallbackInfo<Value>& args) {
    if (event_map.size() == 0)
       initEventMap();

    WrapTrader* obj = new WrapTrader();
    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
}

void WrapTrader::NewInstance(const FunctionCallbackInfo<Value>& args) {
    const unsigned argc = 1;
    void argv[argc] = { args[0] };
    Local<Object> instance = constructor->NewInstance(argc, argv);
    args.GetReturnValue().Set(instance);
}

void WrapTrader::On(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();

    if (args[0]->IsUndefined(isolate) || args[1]->IsUndefined(isolate)) {
        logger_cout("Wrong FunctionCallbackInfo<Value>->event name or function");
        ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>->event name or function")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    
    WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
    
    Local<String> eventName = args[0]->ToString();
    Local<Function> cb = Local<Function>::Cast(args[1]);
    Persistent<Function> unRecoveryCb = Persistent<Function>::New(cb);
    String::AsciiValue eNameAscii(eventName);

    std::map<const char*, int>::iterator eIt = event_map.find(*eNameAscii);
    if (eIt == event_map.end()) {
        logger_cout("System has not register this event");
        ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"System has no register this event")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }

    std::map<int, Persistent<Function> >::iterator cIt = callback_map.find(eIt->second);
    if (cIt != callback_map.end()) {
        logger_cout("Callback is defined before");
        ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Callback is defined before")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }

    callback_map[eIt->second] = unRecoveryCb;    
    obj->uvTrader->On(*eNameAscii,eIt->second, FunCallback);
    args.GetReturnValue().Set(Int32::New(isolate,0));
    return;
}

void WrapTrader::Connect(const FunctionCallbackInfo<Value>& args) {
    std::string log = "wrap_trader Connect------>";
    if (args[0]->IsUndefined(isolate)) {
        logger_cout("Wrong FunctionCallbackInfo<Value>->front addr");
        ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>->front addr")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    if (!args[2]->IsNumber() || !args[3]->IsNumber()) {
        logger_cout("Wrong FunctionCallbackInfo<Value>->public or private topic type");
        ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>->public or private topic type")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }  
    int uuid = -1;
    WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
    if (!args[4]->IsUndefined(isolate) && args[4]->IsFunction()) {
        uuid = ++s_uuid;
        fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[4]));
        std::string _head = std::string(log);
        logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
    }

    Local<String> frontAddr = args[0]->ToString();
    Local<String> szPath = args[1]->IsUndefined(isolate) ? String::NewFromUtf8(isolate,"t") : args[0]->ToString();
    String::AsciiValue addrAscii(frontAddr);
    String::AsciiValue pathAscii(szPath);
    int publicTopicType = args[2]->Int32Value();
    int privateTopicType = args[3]->Int32Value();     
    
    UVConnectField pConnectField; 
    memset(&pConnectField, 0, sizeof(pConnectField));    
    strcpy(pConnectField.front_addr, ((std::string)*addrAscii).c_str());
    strcpy(pConnectField.szPath, ((std::string)*pathAscii).c_str());
    pConnectField.public_topic_type = publicTopicType;
    pConnectField.private_topic_type = privateTopicType;    
    logger_cout(log.append(" ").append((std::string)*addrAscii).append("|").append((std::string)*pathAscii).append("|").append(to_string(publicTopicType)).append("|").append(to_string(privateTopicType)).c_str());
    obj->uvTrader->Connect(&pConnectField, FunRtnCallback, uuid);
    args.GetReturnValue().Set(Undefined(isolate));
    return;
}

void WrapTrader::ReqUserLogin(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    std::string log = "wrap_trader ReqUserLogin------>";
    if (args[0]->IsUndefined(isolate) || args[1]->IsUndefined(isolate) || args[2]->IsUndefined(isolate)) {
        std::string _head = std::string(log);
        logger_cout(_head.append(" Wrong FunctionCallbackInfo<Value>").c_str());
        ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }

    int uuid = -1;
    WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
    if (!args[3]->IsUndefined(isolate) && args[3]->IsFunction()) {
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
    obj->uvTrader->ReqUserLogin(&req, FunRtnCallback, uuid);
    args.GetReturnValue().Set(Undefined(isolate));
    return;
}

void WrapTrader::ReqUserLogout(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    std::string log = "wrap_trader ReqUserLogout------>";

    if (args[0]->IsUndefined(isolate) || args[1]->IsUndefined(isolate)) {
        std::string _head = std::string(log);
        logger_cout(_head.append(" Wrong FunctionCallbackInfo<Value>").c_str());
        ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    int uuid = -1;
    WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
    if (!args[2]->IsUndefined(isolate) && args[2]->IsFunction()) {
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
    obj->uvTrader->ReqUserLogout(&req, FunRtnCallback, uuid);
    args.GetReturnValue().Set(Undefined(isolate));
    return;
}

void WrapTrader::ReqSettlementInfoConfirm(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    std::string log = "wrap_trader ReqSettlementInfoConfirm------>";

    if (args[0]->IsUndefined(isolate) || args[1]->IsUndefined(isolate)) {
        std::string _head = std::string(log);
        logger_cout(_head.append(" Wrong FunctionCallbackInfo<Value>").c_str());
        ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    int uuid = -1;    
    WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
    if (!args[2]->IsUndefined(isolate) && args[2]->IsFunction()) {
        uuid = ++s_uuid;
        fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[2]));
        std::string _head = std::string(log);
        logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
    }     

    Local<String> broker = args[0]->ToString();
    Local<String> investorId = args[1]->ToString();
    String::AsciiValue brokerAscii(broker);
    String::AsciiValue investorIdAscii(investorId);

    CThostFtdcSettlementInfoConfirmField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, ((std::string)*brokerAscii).c_str());
    strcpy(req.InvestorID, ((std::string)*investorIdAscii).c_str());
    logger_cout(log.append(" ").append((std::string)*brokerAscii).append("|").append((std::string)*investorIdAscii).c_str());
    obj->uvTrader->ReqSettlementInfoConfirm(&req, FunRtnCallback, uuid);
    args.GetReturnValue().Set(Undefined(isolate));
    return;
}

void WrapTrader::ReqQryInstrument(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    std::string log = "wrap_trader ReqQryInstrument------>";

    if (args[0]->IsUndefined(isolate)) {
        std::string _head = std::string(log);
        logger_cout(_head.append(" Wrong FunctionCallbackInfo<Value>").c_str());
        ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    int uuid = -1;
    WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
    if (!args[1]->IsUndefined(isolate) && args[1]->IsFunction()) {
        uuid = ++s_uuid;
        fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[1]));
        std::string _head = std::string(log);
        logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
    }

    Local<String> instrumentId = args[0]->ToString();
    String::AsciiValue instrumentIdAscii(instrumentId);

    CThostFtdcQryInstrumentField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.InstrumentID, ((std::string)*instrumentIdAscii).c_str());
    logger_cout(log.append(" ").append((std::string)*instrumentIdAscii).c_str());
    obj->uvTrader->ReqQryInstrument(&req, FunRtnCallback, uuid);
    args.GetReturnValue().Set(Undefined(isolate));
    return;
}

void WrapTrader::ReqQryTradingAccount(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    std::string log = "wrap_trader ReqQryTradingAccount------>";

    if (args[0]->IsUndefined(isolate) || args[1]->IsUndefined(isolate)) {
        std::string _head = std::string(log);
        logger_cout(_head.append(" Wrong FunctionCallbackInfo<Value>").c_str());
        ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    int uuid = -1;
    WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
    if (!args[2]->IsUndefined(isolate) && args[2]->IsFunction()) {
        uuid = ++s_uuid;
        fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[2]));
        std::string _head = std::string(log);
        logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
    }
    Local<String> broker = args[0]->ToString();
    Local<String> investorId = args[1]->ToString();
    String::AsciiValue brokerAscii(broker);
    String::AsciiValue investorIdAscii(investorId);

    CThostFtdcQryTradingAccountField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, ((std::string)*brokerAscii).c_str());
    strcpy(req.InvestorID, ((std::string)*investorIdAscii).c_str());
    logger_cout(log.append(" ").append((std::string)*brokerAscii).append("|").append((std::string)*investorIdAscii).c_str());
    obj->uvTrader->ReqQryTradingAccount(&req, FunRtnCallback, uuid);
    args.GetReturnValue().Set(Undefined(isolate));
    return;
}

void WrapTrader::ReqQryInvestorPosition(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    std::string log = "wrap_trader ReqQryInvestorPosition------>";

    if (args[0]->IsUndefined(isolate) || args[1]->IsUndefined(isolate) || args[2]->IsUndefined(isolate)) {
        std::string _head = std::string(log);
        logger_cout(_head.append(" Wrong FunctionCallbackInfo<Value>").c_str());
        ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    int uuid = -1;
    WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
    if (!args[3]->IsUndefined(isolate) && args[3]->IsFunction()) {
    uuid = ++s_uuid;
    fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[3]));
    std::string _head = std::string(log);
    logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
    }
    Local<String> broker = args[0]->ToString();
    Local<String> investorId = args[1]->ToString();
    Local<String> instrumentId = args[2]->ToString();
    String::AsciiValue brokerAscii(broker);
    String::AsciiValue investorIdAscii(investorId);
    String::AsciiValue instrumentIdAscii(instrumentId);

    CThostFtdcQryInvestorPositionField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, ((std::string)*brokerAscii).c_str());
    strcpy(req.InvestorID, ((std::string)*investorIdAscii).c_str());
    strcpy(req.InstrumentID, ((std::string)*instrumentIdAscii).c_str());

    logger_cout(log.append(" ").append((std::string)*brokerAscii).append("|").append((std::string)*investorIdAscii).append("|").append((std::string)*instrumentIdAscii).c_str());
    obj->uvTrader->ReqQryInvestorPosition(&req, FunRtnCallback, uuid);
    args.GetReturnValue().Set(Undefined(isolate));
    return;
}

void WrapTrader::ReqQryInvestorPositionDetail(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    std::string log = "wrap_trader ReqQryInvestorPositionDetail------>";

    if (args[0]->IsUndefined(isolate) || args[1]->IsUndefined(isolate) || args[2]->IsUndefined(isolate)) {
        std::string _head = std::string(log);
        logger_cout(_head.append(" Wrong FunctionCallbackInfo<Value>").c_str());
        ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    int uuid = -1;
    WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
    if (!args[3]->IsUndefined(isolate) && args[3]->IsFunction()) {
        uuid = ++s_uuid;
        fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[3]));
        std::string _head = std::string(log);
        logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
    }
    Local<String> broker = args[0]->ToString();
    Local<String> investorId = args[1]->ToString();
    Local<String> instrumentId = args[2]->ToString();
    String::AsciiValue brokerAscii(broker);
    String::AsciiValue investorIdAscii(investorId);
    String::AsciiValue instrumentIdAscii(instrumentId);

    CThostFtdcQryInvestorPositionDetailField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, ((std::string)*brokerAscii).c_str());
    strcpy(req.InvestorID, ((std::string)*investorIdAscii).c_str());
    strcpy(req.InstrumentID, ((std::string)*instrumentIdAscii).c_str());

    logger_cout(log.append(" ").append((std::string)*brokerAscii).append("|").append((std::string)*investorIdAscii).append("|").append((std::string)*instrumentIdAscii).c_str());
    obj->uvTrader->ReqQryInvestorPositionDetail(&req, FunRtnCallback, uuid);
    args.GetReturnValue().Set(Undefined(isolate));
    return;
}

void WrapTrader::ReqOrderInsert(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    std::string log = "wrap_trader ReqOrderInsert------>";

    if (args[0]->IsUndefined(isolate) || !args[0]->IsObject()) {
        std::string _head = std::string(log);
        logger_cout(_head.append(" Wrong FunctionCallbackInfo<Value>").c_str());
        ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    int uuid = -1;
    WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
    if (!args[1]->IsUndefined(isolate) && args[1]->IsFunction()) {
        uuid = ++s_uuid;
        fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[1]));
        std::string _head = std::string(log);
        logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
    }
    Local<Object> jsonObj = args[0]->ToObject();
    Local<Value> brokerId = jsonObj->Get(v8::String::NewFromUtf8(isolate,"brokerId"));
    if (brokerId->IsUndefined(isolate)) {
        ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>->brokerId")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    String::AsciiValue brokerId_(brokerId->ToString());
    Local<Value> investorId = jsonObj->Get(v8::String::NewFromUtf8(isolate,"investorId"));
    if (investorId->IsUndefined(isolate)) {
        ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>->investorId")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    String::AsciiValue investorId_(investorId->ToString());
    Local<Value> instrumentId = jsonObj->Get(v8::String::NewFromUtf8(isolate,"instrumentId"));
    if (instrumentId->IsUndefined(isolate)) {
        ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>->instrumentId")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    String::AsciiValue instrumentId_(instrumentId->ToString());
    Local<Value> priceType = jsonObj->Get(v8::String::NewFromUtf8(isolate,"priceType"));
    if (priceType->IsUndefined(isolate)) {
        ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>->priceType")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    String::AsciiValue priceType_(priceType->ToString());
    Local<Value> direction = jsonObj->Get(v8::String::NewFromUtf8(isolate,"direction"));
    if (direction->IsUndefined(isolate)) {
        ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>->direction")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    String::AsciiValue direction_(direction->ToString());
    Local<Value> combOffsetFlag = jsonObj->Get(v8::String::NewFromUtf8(isolate,"combOffsetFlag"));
    if (combOffsetFlag->IsUndefined(isolate)) {
        ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>->combOffsetFlag")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    String::AsciiValue combOffsetFlag_(combOffsetFlag->ToString());
    Local<Value> combHedgeFlag = jsonObj->Get(v8::String::NewFromUtf8(isolate,"combHedgeFlag"));
    if (combHedgeFlag->IsUndefined(isolate)) {
        ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>->combHedgeFlag")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    String::AsciiValue combHedgeFlag_(combHedgeFlag->ToString());
    Local<Value> vlimitPrice = jsonObj->Get(v8::String::NewFromUtf8(isolate,"limitPrice"));
    if (vlimitPrice->IsUndefined(isolate)) {
        ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>->limitPrice")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    double limitPrice = vlimitPrice->NumberValue();
    Local<Value> vvolumeTotalOriginal = jsonObj->Get(v8::String::NewFromUtf8(isolate,"volumeTotalOriginal"));
    if (vvolumeTotalOriginal->IsUndefined(isolate)) {
        ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>->volumeTotalOriginal")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    int32_t volumeTotalOriginal = vvolumeTotalOriginal->Int32Value();
    Local<Value> timeCondition = jsonObj->Get(v8::String::NewFromUtf8(isolate,"timeCondition"));
    if (timeCondition->IsUndefined(isolate)) {
        ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>->timeCondition")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    String::AsciiValue timeCondition_(timeCondition->ToString());
    Local<Value> volumeCondition = jsonObj->Get(v8::String::NewFromUtf8(isolate,"volumeCondition"));
    if (volumeCondition->IsUndefined(isolate)) {
        ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>->volumeCondition")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    String::AsciiValue volumeCondition_(volumeCondition->ToString());
    Local<Value> vminVolume = jsonObj->Get(v8::String::NewFromUtf8(isolate,"minVolume"));
    if (vminVolume->IsUndefined(isolate)) {
        ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>->minVolume")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    int32_t minVolume = vminVolume->Int32Value();
    Local<Value> forceCloseReason = jsonObj->Get(v8::String::NewFromUtf8(isolate,"forceCloseReason"));
    if (forceCloseReason->IsUndefined(isolate)) {
        ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>->forceCloseReason")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    String::AsciiValue forceCloseReason_(forceCloseReason->ToString());
    Local<Value> visAutoSuspend = jsonObj->Get(v8::String::NewFromUtf8(isolate,"isAutoSuspend"));
    if (visAutoSuspend->IsUndefined(isolate)) {
        ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>->isAutoSuspend")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    int32_t isAutoSuspend = visAutoSuspend->Int32Value();
    Local<Value> vuserForceClose = jsonObj->Get(v8::String::NewFromUtf8(isolate,"userForceClose"));
    if (vuserForceClose->IsUndefined(isolate)) {
        ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>->userForceClose")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    int32_t userForceClose = vuserForceClose->Int32Value();

    CThostFtdcInputOrderField req;
    memset(&req, 0, sizeof(req));
    log.append(" ");

    Local<Value> orderRef = jsonObj->Get(v8::String::NewFromUtf8(isolate,"orderRef"));
    if (!orderRef->IsUndefined(isolate)) {
        String::AsciiValue orderRef_(orderRef->ToString());
        strcpy(req.OrderRef, ((std::string)*orderRef_).c_str());  
        log.append("orderRef:").append((std::string)*orderRef_).append("|");
    }

    Local<Value> vstopPrice = jsonObj->Get(v8::String::NewFromUtf8(isolate,"stopPrice"));
    if(!vstopPrice->IsUndefined(isolate)){
        double stopPrice = vstopPrice->NumberValue();
        req.StopPrice = stopPrice;
        log.append("stopPrice:").append(to_string(stopPrice)).append("|");
    }
    Local<Value> contingentCondition = jsonObj->Get(v8::String::NewFromUtf8(isolate,"contingentCondition"));
    if (!contingentCondition->IsUndefined(isolate)) {
        String::AsciiValue contingentCondition_(contingentCondition->ToString());
        req.ContingentCondition = ((std::string)*contingentCondition_)[0];
        log.append("contingentCondition:").append((std::string)*contingentCondition_).append("|");
    }

    strcpy(req.BrokerID, ((std::string)*brokerId_).c_str());
    strcpy(req.InvestorID, ((std::string)*investorId_).c_str());
    strcpy(req.InstrumentID, ((std::string)*instrumentId_).c_str());
    req.OrderPriceType = ((std::string)*priceType_)[0];
    req.Direction = ((std::string)*direction_)[0];
    req.CombOffsetFlag[0] = ((std::string)*combOffsetFlag_)[0];
    req.CombHedgeFlag[0] = ((std::string)*combHedgeFlag_)[0];
    req.LimitPrice = limitPrice;
    req.VolumeTotalOriginal = volumeTotalOriginal;
    req.TimeCondition = ((std::string)*timeCondition_)[0];
    req.VolumeCondition = ((std::string)*volumeCondition_)[0];
    req.MinVolume = minVolume;
    req.ForceCloseReason = ((std::string)*forceCloseReason_)[0];
    req.IsAutoSuspend = isAutoSuspend;
    req.UserForceClose = userForceClose;
    logger_cout(log.
        append("brokerID:").append((std::string)*brokerId_).append("|").
    append("investorID:").append((std::string)*investorId_).append("|").
    append("instrumentID:").append((std::string)*instrumentId_).append("|").
    append("priceType:").append((std::string)*priceType_).append("|").
    append("direction:").append((std::string)*direction_).append("|").
    append("comboffsetFlag:").append((std::string)*combOffsetFlag_).append("|").
    append("combHedgeFlag:").append((std::string)*combHedgeFlag_).append("|").
    append("limitPrice:").append(to_string(limitPrice)).append("|").
    append("volumnTotalOriginal:").append(to_string(volumeTotalOriginal)).append("|").
    append("timeCondition:").append((std::string)*timeCondition_).append("|").
    append("volumeCondition:").append((std::string)*volumeCondition_).append("|").
    append("minVolume:").append(to_string(minVolume)).append("|").
    append("forceCloseReason:").append((std::string)*forceCloseReason_).append("|").
    append("isAutoSuspend:").append(to_string(isAutoSuspend)).append("|").
    append("useForceClose:").append(to_string(userForceClose)).append("|").c_str());
    obj->uvTrader->ReqOrderInsert(&req, FunRtnCallback, uuid);
    args.GetReturnValue().Set(Undefined(isolate));
    return;
}

void WrapTrader::ReqOrderAction(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    std::string log = "wrap_trader ReqOrderAction------>";

    if (args[0]->IsUndefined(isolate) || !args[0]->IsObject()) {
        std::string _head = std::string(log);
        logger_cout(_head.append(" Wrong FunctionCallbackInfo<Value>").c_str());
        ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    int uuid = -1;
    WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());

    if (!args[1]->IsUndefined(isolate) && args[1]->IsFunction()) {
        uuid = ++s_uuid;
        fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[1]));
        std::string _head = std::string(log);
        logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
    }

    Local<Object> jsonObj = args[0]->ToObject();
    Local<Value> vbrokerId = jsonObj->Get(v8::String::NewFromUtf8(isolate,"brokerId"));
    if (vbrokerId->IsUndefined(isolate)) {
        ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>->brokerId")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    String::AsciiValue brokerId_(vbrokerId->ToString());
    Local<Value> vinvestorId = jsonObj->Get(v8::String::NewFromUtf8(isolate,"investorId"));
    if (vinvestorId->IsUndefined(isolate)) {
        ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>->investorId")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    String::AsciiValue investorId_(vinvestorId->ToString());
    Local<Value> vinstrumentId = jsonObj->Get(v8::String::NewFromUtf8(isolate,"instrumentId"));
    if (vinstrumentId->IsUndefined(isolate)) {
        ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>->instrumentId")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    String::AsciiValue instrumentId_(vinstrumentId->ToString());
    Local<Value> vactionFlag = jsonObj->Get(v8::String::NewFromUtf8(isolate,"actionFlag"));
    if (vactionFlag->IsUndefined(isolate)) {
        ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>->actionFlag")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    int32_t actionFlag = vactionFlag->Int32Value();

    CThostFtdcInputOrderActionField req;
    memset(&req, 0, sizeof(req));

    log.append(" ");
    Local<Value> vorderRef = jsonObj->Get(v8::String::NewFromUtf8(isolate,"orderRef"));
    if (!vorderRef->IsUndefined(isolate)) {
        String::AsciiValue orderRef_(vorderRef->ToString());
        strcpy(req.OrderRef, ((std::string)*orderRef_).c_str());
        log.append((std::string)*orderRef_).append("|");
    }
    Local<Value> vfrontId = jsonObj->Get(v8::String::NewFromUtf8(isolate,"frontId"));
    if (!vfrontId->IsUndefined(isolate)) {
        int32_t frontId = vfrontId->Int32Value();
        req.FrontID = frontId;
        log.append(to_string(frontId)).append("|");
    }
    Local<Value> vsessionId = jsonObj->Get(v8::String::NewFromUtf8(isolate,"sessionId"));
    if (!vsessionId->IsUndefined(isolate)) {
        int32_t sessionId = vsessionId->Int32Value();
        req.SessionID = sessionId;
        log.append(to_string(sessionId)).append("|");
    }
    Local<Value> vexchangeID = jsonObj->Get(v8::String::NewFromUtf8(isolate,"exchangeID"));
    if (!vexchangeID->IsUndefined(isolate)) {
        String::AsciiValue exchangeID_(vexchangeID->ToString());
        strcpy(req.ExchangeID, ((std::string)*exchangeID_).c_str());
        log.append((std::string)*exchangeID_).append("|");
    }
    Local<Value> vorderSysID = jsonObj->Get(v8::String::NewFromUtf8(isolate,"orderSysID"));
    if (vorderSysID->IsUndefined(isolate)) {
        String::AsciiValue orderSysID_(vorderSysID->ToString());
        strcpy(req.OrderSysID, ((std::string)*orderSysID_).c_str());
        log.append((std::string)*orderSysID_).append("|");
    }

    strcpy(req.BrokerID, ((std::string)*brokerId_).c_str());
    strcpy(req.InvestorID, ((std::string)*investorId_).c_str());
    req.ActionFlag = actionFlag;
    strcpy(req.InstrumentID, ((std::string)*instrumentId_).c_str());
    logger_cout(log.
    append((std::string)*brokerId_).append("|").
    append((std::string)*investorId_).append("|").
    append((std::string)*instrumentId_).append("|").
    append(to_string(actionFlag)).append("|").c_str());

    obj->uvTrader->ReqOrderAction(&req, FunRtnCallback, uuid);
    args.GetReturnValue().Set(Undefined(isolate));
    return;
}

void WrapTrader::ReqQryInstrumentMarginRate(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    std::string log = "wrap_trader ReqQryInstrumentMarginRate------>";

    if (args[0]->IsUndefined(isolate) || args[1]->IsUndefined(isolate) || args[2]->IsUndefined(isolate) || args[3]->IsUndefined(isolate)) {
    std::string _head = std::string(log);
    logger_cout(_head.append(" Wrong FunctionCallbackInfo<Value>").c_str());
    ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>")));
            args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    int uuid = -1;
    WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());

    if (!args[4]->IsUndefined(isolate) && args[4]->IsFunction()) {
    uuid = ++s_uuid;
    fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[4]));
    std::string _head = std::string(log);
    logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
    }

    Local<String> broker = args[0]->ToString();
    Local<String> investorId = args[1]->ToString();
    Local<String> instrumentId = args[2]->ToString();
    int32_t hedgeFlag = args[3]->Int32Value();
    String::AsciiValue brokerAscii(broker);
    String::AsciiValue investorIdAscii(investorId);
    String::AsciiValue instrumentIdAscii(instrumentId);

    CThostFtdcQryInstrumentMarginRateField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, ((std::string)*brokerAscii).c_str());
    strcpy(req.InvestorID, ((std::string)*investorIdAscii).c_str());
    strcpy(req.InstrumentID, ((std::string)*instrumentIdAscii).c_str());
    req.HedgeFlag = hedgeFlag;
    logger_cout(log.append(" ").
    append((std::string)*brokerAscii).append("|").
    append((std::string)*investorIdAscii).append("|").
    append((std::string)*instrumentIdAscii).append("|").
    append(to_string(hedgeFlag)).append("|").c_str());     

    obj->uvTrader->ReqQryInstrumentMarginRate(&req, FunRtnCallback, uuid);
    args.GetReturnValue().Set(Undefined(isolate));
    return;
}

void WrapTrader::ReqQryDepthMarketData(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    std::string log = "wrap_trader ReqQryDepthMarketData------>";

    if (args[0]->IsUndefined(isolate)) {
        std::string _head = std::string(log);
        logger_cout(_head.append(" Wrong FunctionCallbackInfo<Value>").c_str());
        ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    int uuid = -1;
    WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
    if (!args[1]->IsUndefined(isolate) && args[1]->IsFunction()) {
        uuid = ++s_uuid;
        fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[1]));
        std::string _head = std::string(log);
        logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
    }

    Local<String> instrumentId = args[0]->ToString();
    String::AsciiValue instrumentIdAscii(instrumentId);

    CThostFtdcQryDepthMarketDataField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.InstrumentID, ((std::string)*instrumentIdAscii).c_str());
    logger_cout(log.append(" ").
    append((std::string)*instrumentIdAscii).append("|").c_str());
    obj->uvTrader->ReqQryDepthMarketData(&req, FunRtnCallback, uuid);
    args.GetReturnValue().Set(Undefined(isolate));
    return;
}

void WrapTrader::ReqQrySettlementInfo(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    std::string log = "wrap_trader ReqQrySettlementInfo------>";

    if (args[0]->IsUndefined(isolate) || args[1]->IsUndefined(isolate) || args[2]->IsUndefined(isolate)) {
    std::string _head = std::string(log);
    logger_cout(_head.append(" Wrong FunctionCallbackInfo<Value>").c_str());
    ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong FunctionCallbackInfo<Value>")));
            args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    int uuid = -1;
    WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
    if (!args[3]->IsUndefined(isolate) && args[3]->IsFunction()) {
    uuid = ++s_uuid;
    fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[3]));
    std::string _head = std::string(log);
    logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
    }

    Local<String> broker = args[0]->ToString();
    Local<String> investorId = args[1]->ToString();
    Local<String> tradingDay = args[2]->ToString();
    String::AsciiValue brokerAscii(broker);
    String::AsciiValue investorIdAscii(investorId);
    String::AsciiValue tradingDayAscii(tradingDay);

    CThostFtdcQrySettlementInfoField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, ((std::string)*brokerAscii).c_str());
    strcpy(req.InvestorID, ((std::string)*investorIdAscii).c_str());
    strcpy(req.TradingDay, ((std::string)*tradingDayAscii).c_str());
    logger_cout(log.append(" ").
    append((std::string)*brokerAscii).append("|").
    append((std::string)*investorIdAscii).append("|").
    append((std::string)*tradingDayAscii).append("|").c_str());

    obj->uvTrader->ReqQrySettlementInfo(&req, FunRtnCallback, uuid);
    args.GetReturnValue().Set(Undefined(isolate));
    return;
}

void WrapTrader::Disposed(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
    obj->uvTrader->Disconnect();    
    std::map<int, Persistent<Function> >::iterator callback_it = callback_map.begin();
    while (callback_it != callback_map.end()) {
    callback_it->second.Dispose();
    callback_it++;
    }
    event_map.clear();
    callback_map.clear();
    fun_rtncb_map.clear();
    delete obj->uvTrader;
    obj->uvTrader = NULL;
    logger_cout("wrap_trader Disposed------>wrap disposed");
    args.GetReturnValue().Set(Undefined(isolate));
    return;
}

void WrapTrader::GetTradingDay(const FunctionCallbackInfo<Value>& args){
    Isolate* isolate = args.GetIsolate();
    WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
    const char* tradingDay = obj->uvTrader->GetTradingDay();
    return scope.Close(String::NewFromUtf8(isolate,tradingDay));
}

void WrapTrader::FunCallback(CbRtnField *data) {
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
                  Local<Value> argv[1] = { Int32::New(data->nReason) };
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
    case T_ON_RSPINFOCONFIRM:
    {
                Local<Value> argv[4];
                pkg_cb_confirm(data, argv);
                cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);
                break;
    }
    case T_ON_RSPINSERT:
    {
               Local<Value> argv[4];
               pkg_cb_orderinsert(data, argv);
               cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);
               break;
    }
    case T_ON_ERRINSERT:
    {
               Local<Value> argv[2];
               pkg_cb_errorderinsert(data, argv);
               cIt->second->Call(Context::GetCurrent()->Global(), 2, argv);
               break;
    }
    case T_ON_RSPACTION:
    {
               Local<Value> argv[4];
               pkg_cb_orderaction(data, argv);
               cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);
               break;
    }
    case T_ON_ERRACTION:
    {
               Local<Value> argv[2];
               pkg_cb_errorderaction(data, argv);
               cIt->second->Call(Context::GetCurrent()->Global(), 2, argv);

               break;
    }
    case T_ON_RQORDER:
    {
             Local<Value> argv[4];
             pkg_cb_rspqryorder(data, argv);
             cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);
             break;
    }
    case T_ON_RTNORDER:
    {
              Local<Value> argv[1];
              pkg_cb_rtnorder(data, argv);
              cIt->second->Call(Context::GetCurrent()->Global(), 1, argv);

              break;
    }
    case T_ON_RQTRADE:
    {
             Local<Value> argv[4];
             pkg_cb_rqtrade(data, argv);
             cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);

             break;
    }
    case T_ON_RTNTRADE:
    {
              Local<Value> argv[1];
              pkg_cb_rtntrade(data, argv);
              cIt->second->Call(Context::GetCurrent()->Global(), 1, argv);

              break;
    }
    case T_ON_RQINVESTORPOSITION:
    {
                    Local<Value> argv[4];
                    pkg_cb_rqinvestorposition(data, argv);
                    cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);

                    break;
    }
    case T_ON_RQINVESTORPOSITIONDETAIL:
    {
                      Local<Value> argv[4];
                      pkg_cb_rqinvestorpositiondetail(data, argv);
                      cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);

                      break;
    }
    case T_ON_RQTRADINGACCOUNT:
    {
                  Local<Value> argv[4];
                  pkg_cb_rqtradingaccount(data, argv);
                  cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);

                  break;
    }
    case T_ON_RQINSTRUMENT:
    {
                  Local<Value> argv[4];
                  pkg_cb_rqinstrument(data, argv);
                  cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);

                  break;
    }
    case T_ON_RQDEPTHMARKETDATA:
    {
                   Local<Value> argv[4];
                   pkg_cb_rqdepthmarketdata(data, argv);
                   cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);

                   break;
    }
    case T_ON_RQSETTLEMENTINFO:
    {
                  Local<Value> argv[4];
                  pkg_cb_rqsettlementinfo(data, argv);
                  cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);
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

void WrapTrader::FunRtnCallback(int result, void* baton) {
    Isolate* isolate = args.GetIsolate();
    LookupCtpApiBaton* tmp = static_cast<LookupCtpApiBaton*>(baton);     
    if (tmp->uuid != -1) {
    std::map<int, Persistent<Function> >::iterator it = fun_rtncb_map.find(tmp->uuid);
    Local<Value> argv[2] = { Local<Value>::New(Int32::New(tmp->nResult)),Local<Value>::New(Int32::New(tmp->iRequestID)) };
    it->second->Call(Context::GetCurrent()->Global(), 2, argv);
    it->second.Dispose();
    fun_rtncb_map.erase(tmp->uuid);          
    }
    scope.Close(Undefined(isolate));
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void WrapTrader::pkg_cb_userlogin(CbRtnField* data, Local<Value>*cbArray) {
    *cbArray = Int32::New(data->nRequestID);
    *(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
    if (data->rtnField){  
        CThostFtdcRspUserLoginField* pRspUserLogin = static_cast<CThostFtdcRspUserLoginField*>(data->rtnField);
    Local<Object> jsonRtn = Object::New();
    jsonRtn->Set(String::NewFromUtf8(isolate,"TradingDay"), String::NewFromUtf8(isolate,pRspUserLogin->TradingDay));
    jsonRtn->Set(String::NewFromUtf8(isolate,"LoginTime"), String::NewFromUtf8(isolate,pRspUserLogin->LoginTime));
    jsonRtn->Set(String::NewFromUtf8(isolate,"BrokerID"), String::NewFromUtf8(isolate,pRspUserLogin->BrokerID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"UserID"), String::NewFromUtf8(isolate,pRspUserLogin->UserID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"SystemName"), String::NewFromUtf8(isolate,pRspUserLogin->SystemName));
    jsonRtn->Set(String::NewFromUtf8(isolate,"FrontID"), Int32::New(pRspUserLogin->FrontID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"SessionID"), Int32::New(pRspUserLogin->SessionID));
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
    
    *(cbArray + 3) = pkg_rspinfo(data->rspInfo);
    return;
}
void WrapTrader::pkg_cb_userlogout(CbRtnField* data, Local<Value>*cbArray) {
    *cbArray = Int32::New(data->nRequestID);
    *(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
    if (data->rtnField){ 
        CThostFtdcRspUserLoginField* pRspUserLogin = static_cast<CThostFtdcRspUserLoginField*>(data->rtnField);

    Local<Object> jsonRtn = Object::New();
    jsonRtn->Set(String::NewFromUtf8(isolate,"BrokerID"), String::NewFromUtf8(isolate,pRspUserLogin->BrokerID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"UserID"), String::NewFromUtf8(isolate,pRspUserLogin->UserID));
    *(cbArray + 2) = jsonRtn;
    }
    else {
    *(cbArray + 2) = Local<Value>::New(Undefined(isolate));
    }
    *(cbArray + 3) = pkg_rspinfo(data->rspInfo);
    return;
}
void WrapTrader::pkg_cb_confirm(CbRtnField* data, Local<Value>*cbArray) {
    *cbArray = Int32::New(data->nRequestID);
    *(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
    if (data->rtnField){ 
        CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm = static_cast<CThostFtdcSettlementInfoConfirmField*>(data->rtnField);
    Local<Object> jsonRtn = Object::New();
    jsonRtn->Set(String::NewFromUtf8(isolate,"BrokerID"), String::NewFromUtf8(isolate,pSettlementInfoConfirm->BrokerID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"InvestorID"), String::NewFromUtf8(isolate,pSettlementInfoConfirm->InvestorID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ConfirmDate"), String::NewFromUtf8(isolate,pSettlementInfoConfirm->ConfirmDate));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ConfirmTime"), String::NewFromUtf8(isolate,pSettlementInfoConfirm->ConfirmTime));
    *(cbArray + 2) = jsonRtn;
    }
    else {
    *(cbArray + 2) = Local<Value>::New(Undefined(isolate));
    }
    *(cbArray + 3) = pkg_rspinfo(data->rspInfo);
    return;
}
void WrapTrader::pkg_cb_orderinsert(CbRtnField* data, Local<Value>*cbArray) {
    *cbArray = Int32::New(data->nRequestID);
    *(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
    if (data->rtnField){ 
        CThostFtdcInputOrderField* pInputOrder = static_cast<CThostFtdcInputOrderField*>(data->rtnField);
    Local<Object> jsonRtn = Object::New();
    jsonRtn->Set(String::NewFromUtf8(isolate,"BrokerID"), String::NewFromUtf8(isolate,pInputOrder->BrokerID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"InvestorID"), String::NewFromUtf8(isolate,pInputOrder->InvestorID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"InstrumentID"), String::NewFromUtf8(isolate,pInputOrder->InstrumentID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OrderRef"), String::NewFromUtf8(isolate,pInputOrder->OrderRef));
    jsonRtn->Set(String::NewFromUtf8(isolate,"UserID"), String::NewFromUtf8(isolate,pInputOrder->UserID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OrderPriceType"), String::NewFromUtf8(isolate,charto_string(pInputOrder->OrderPriceType).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"Direction"), String::NewFromUtf8(isolate,charto_string(pInputOrder->Direction).c_str()));  //var charval = String.fromCharCode(asciival);
    jsonRtn->Set(String::NewFromUtf8(isolate,"CombOffsetFlag"), String::NewFromUtf8(isolate,pInputOrder->CombOffsetFlag));
    jsonRtn->Set(String::NewFromUtf8(isolate,"CombHedgeFlag"), String::NewFromUtf8(isolate,pInputOrder->CombHedgeFlag));
    jsonRtn->Set(String::NewFromUtf8(isolate,"LimitPrice"), Number::New(pInputOrder->LimitPrice));
    jsonRtn->Set(String::NewFromUtf8(isolate,"VolumeTotalOriginal"), Int32::New(pInputOrder->VolumeTotalOriginal));
    jsonRtn->Set(String::NewFromUtf8(isolate,"TimeCondition"), String::NewFromUtf8(isolate,charto_string(pInputOrder->TimeCondition).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"GTDDate"), String::NewFromUtf8(isolate,pInputOrder->GTDDate));
    jsonRtn->Set(String::NewFromUtf8(isolate,"VolumeCondition"), String::NewFromUtf8(isolate,charto_string(pInputOrder->VolumeCondition).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"MinVolume"), Int32::New(pInputOrder->MinVolume));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ContingentCondition"), String::NewFromUtf8(isolate,charto_string(pInputOrder->ContingentCondition).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"StopPrice"), Number::New(pInputOrder->StopPrice));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ForceCloseReason"), String::NewFromUtf8(isolate,charto_string(pInputOrder->ForceCloseReason).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"IsAutoSuspend"), Int32::New(pInputOrder->IsAutoSuspend));
    jsonRtn->Set(String::NewFromUtf8(isolate,"BusinessUnit"), String::NewFromUtf8(isolate,pInputOrder->BusinessUnit));
    jsonRtn->Set(String::NewFromUtf8(isolate,"RequestID"), Int32::New(pInputOrder->RequestID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"UserForceClose"), Int32::New(pInputOrder->UserForceClose));
    jsonRtn->Set(String::NewFromUtf8(isolate,"IsSwapOrder"), Int32::New(pInputOrder->IsSwapOrder));
    *(cbArray + 2) = jsonRtn;
    }
    else {
    *(cbArray + 2) = Local<Value>::New(Undefined(isolate));
    }
    *(cbArray + 3) = pkg_rspinfo(data->rspInfo);
    return;
}
void WrapTrader::pkg_cb_errorderinsert(CbRtnField* data, Local<Value>*cbArray) {
    if (data->rtnField){ 
        CThostFtdcInputOrderField* pInputOrder = static_cast<CThostFtdcInputOrderField*>(data->rtnField);
    Local<Object> jsonRtn = Object::New();
    jsonRtn->Set(String::NewFromUtf8(isolate,"BrokerID"), String::NewFromUtf8(isolate,pInputOrder->BrokerID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"InvestorID"), String::NewFromUtf8(isolate,pInputOrder->InvestorID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"InstrumentID"), String::NewFromUtf8(isolate,pInputOrder->InstrumentID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OrderRef"), String::NewFromUtf8(isolate,pInputOrder->OrderRef));
    jsonRtn->Set(String::NewFromUtf8(isolate,"UserID"), String::NewFromUtf8(isolate,pInputOrder->UserID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OrderPriceType"), String::NewFromUtf8(isolate,charto_string(pInputOrder->OrderPriceType).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"Direction"), String::NewFromUtf8(isolate,charto_string(pInputOrder->Direction).c_str()));  //var charval = String.fromCharCode(asciival);
    jsonRtn->Set(String::NewFromUtf8(isolate,"CombOffsetFlag"), String::NewFromUtf8(isolate,pInputOrder->CombOffsetFlag));
    jsonRtn->Set(String::NewFromUtf8(isolate,"CombHedgeFlag"), String::NewFromUtf8(isolate,pInputOrder->CombHedgeFlag));
    jsonRtn->Set(String::NewFromUtf8(isolate,"LimitPrice"), Number::New(pInputOrder->LimitPrice));
    jsonRtn->Set(String::NewFromUtf8(isolate,"VolumeTotalOriginal"), Int32::New(pInputOrder->VolumeTotalOriginal));
    jsonRtn->Set(String::NewFromUtf8(isolate,"TimeCondition"), String::NewFromUtf8(isolate,charto_string(pInputOrder->TimeCondition).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"GTDDate"), String::NewFromUtf8(isolate,pInputOrder->GTDDate));
    jsonRtn->Set(String::NewFromUtf8(isolate,"VolumeCondition"), String::NewFromUtf8(isolate,charto_string(pInputOrder->VolumeCondition).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"MinVolume"), Int32::New(pInputOrder->MinVolume));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ContingentCondition"), String::NewFromUtf8(isolate,charto_string(pInputOrder->ContingentCondition).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"StopPrice"), Number::New(pInputOrder->StopPrice));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ForceCloseReason"), String::NewFromUtf8(isolate,charto_string(pInputOrder->ForceCloseReason).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"IsAutoSuspend"), Int32::New(pInputOrder->IsAutoSuspend));
    jsonRtn->Set(String::NewFromUtf8(isolate,"BusinessUnit"), String::NewFromUtf8(isolate,pInputOrder->BusinessUnit));
    jsonRtn->Set(String::NewFromUtf8(isolate,"RequestID"), Int32::New(pInputOrder->RequestID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"UserForceClose"), Int32::New(pInputOrder->UserForceClose));
    jsonRtn->Set(String::NewFromUtf8(isolate,"IsSwapOrder"), Int32::New(pInputOrder->IsSwapOrder));
    *cbArray = jsonRtn;
    }
    else {
    *cbArray = Local<Value>::New(Undefined(isolate));
    }
    *(cbArray + 1) = pkg_rspinfo(data->rspInfo);
    return;
}
void WrapTrader::pkg_cb_orderaction(CbRtnField* data, Local<Value>*cbArray) {
    *cbArray = Int32::New(data->nRequestID);
    *(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
    if (data->rtnField){ 
        CThostFtdcInputOrderActionField* pInputOrderAction = static_cast<CThostFtdcInputOrderActionField*>(data->rtnField);
    Local<Object> jsonRtn = Object::New();
    jsonRtn->Set(String::NewFromUtf8(isolate,"BrokerID"), String::NewFromUtf8(isolate,pInputOrderAction->BrokerID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"InvestorID"), String::NewFromUtf8(isolate,pInputOrderAction->InvestorID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OrderActionRef"), Int32::New(pInputOrderAction->OrderActionRef));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OrderRef"), String::NewFromUtf8(isolate,pInputOrderAction->OrderRef));
    jsonRtn->Set(String::NewFromUtf8(isolate,"RequestID"), Int32::New(pInputOrderAction->RequestID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"FrontID"), Int32::New(pInputOrderAction->FrontID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"SessionID"), Int32::New(pInputOrderAction->SessionID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ExchangeID"), String::NewFromUtf8(isolate,pInputOrderAction->ExchangeID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OrderSysID"), String::NewFromUtf8(isolate,pInputOrderAction->OrderSysID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ActionFlag"), String::NewFromUtf8(isolate,charto_string(pInputOrderAction->ActionFlag).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"LimitPrice"), Number::New(pInputOrderAction->LimitPrice));
    jsonRtn->Set(String::NewFromUtf8(isolate,"VolumeChange"), Int32::New(pInputOrderAction->VolumeChange));
    jsonRtn->Set(String::NewFromUtf8(isolate,"UserID"), String::NewFromUtf8(isolate,pInputOrderAction->UserID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"InstrumentID"), String::NewFromUtf8(isolate,pInputOrderAction->InstrumentID));
    *(cbArray + 2) = jsonRtn;
    }
    else {
    *(cbArray + 2) = Local<Value>::New(Undefined(isolate));
    }
    *(cbArray + 3) = pkg_rspinfo(data->rspInfo);
    return;
}
void WrapTrader::pkg_cb_errorderaction(CbRtnField* data, Local<Value>*cbArray) {
    if (data->rtnField){ 
        CThostFtdcOrderActionField* pOrderAction = static_cast<CThostFtdcOrderActionField*>(data->rtnField);
    Local<Object> jsonRtn = Object::New();
    jsonRtn->Set(String::NewFromUtf8(isolate,"BrokerID"), String::NewFromUtf8(isolate,pOrderAction->BrokerID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"InvestorID"), String::NewFromUtf8(isolate,pOrderAction->InvestorID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OrderActionRef"), Int32::New(pOrderAction->OrderActionRef));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OrderRef"), String::NewFromUtf8(isolate,pOrderAction->OrderRef));
    jsonRtn->Set(String::NewFromUtf8(isolate,"RequestID"), Int32::New(pOrderAction->RequestID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"FrontID"), Int32::New(pOrderAction->FrontID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"SessionID"), Int32::New(pOrderAction->SessionID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ExchangeID"), String::NewFromUtf8(isolate,pOrderAction->ExchangeID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OrderSysID"), String::NewFromUtf8(isolate,pOrderAction->OrderSysID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ActionFlag"), String::NewFromUtf8(isolate,charto_string(pOrderAction->ActionFlag).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"LimitPrice"), Number::New(pOrderAction->LimitPrice));
    jsonRtn->Set(String::NewFromUtf8(isolate,"VolumeChange"), Int32::New(pOrderAction->VolumeChange));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ActionDate"), String::NewFromUtf8(isolate,pOrderAction->ActionDate));
    jsonRtn->Set(String::NewFromUtf8(isolate,"TraderID"), String::NewFromUtf8(isolate,pOrderAction->TraderID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"InstallID"), Int32::New(pOrderAction->InstallID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OrderLocalID"), String::NewFromUtf8(isolate,pOrderAction->OrderLocalID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ActionLocalID"), String::NewFromUtf8(isolate,pOrderAction->ActionLocalID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ParticipantID"), String::NewFromUtf8(isolate,pOrderAction->ParticipantID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ClientID"), String::NewFromUtf8(isolate,pOrderAction->ClientID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"BusinessUnit"), String::NewFromUtf8(isolate,pOrderAction->BusinessUnit));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OrderActionStatus"), String::NewFromUtf8(isolate,charto_string(pOrderAction->OrderActionStatus).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"UserID"), String::NewFromUtf8(isolate,pOrderAction->UserID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"StatusMsg"), String::NewFromUtf8(isolate,pOrderAction->StatusMsg));
    jsonRtn->Set(String::NewFromUtf8(isolate,"InstrumentID"), String::NewFromUtf8(isolate,pOrderAction->InstrumentID));
    *cbArray = jsonRtn;
    }
    else {
    *cbArray = Local<Value>::New(Undefined(isolate));
    }
    *(cbArray + 1) = pkg_rspinfo(data->rspInfo);
    return;
}
void WrapTrader::pkg_cb_rspqryorder(CbRtnField* data, Local<Value>*cbArray) {
    *cbArray = Int32::New(data->nRequestID);
    *(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
    if (data->rtnField){ 
        CThostFtdcOrderField* pOrder = static_cast<CThostFtdcOrderField*>(data->rtnField);
    Local<Object> jsonRtn = Object::New();
    jsonRtn->Set(String::NewFromUtf8(isolate,"BrokerID"), String::NewFromUtf8(isolate,pOrder->BrokerID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"InvestorID"), String::NewFromUtf8(isolate,pOrder->InvestorID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"InstrumentID"), String::NewFromUtf8(isolate,pOrder->InstrumentID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OrderRef"), String::NewFromUtf8(isolate,pOrder->OrderRef));
    jsonRtn->Set(String::NewFromUtf8(isolate,"UserID"), String::NewFromUtf8(isolate,pOrder->UserID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OrderPriceType"), String::NewFromUtf8(isolate,charto_string(pOrder->OrderPriceType).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"Direction"), String::NewFromUtf8(isolate,charto_string(pOrder->Direction).c_str()));  //var charval = String.fromCharCode(asciival);
    jsonRtn->Set(String::NewFromUtf8(isolate,"CombOffsetFlag"), String::NewFromUtf8(isolate,pOrder->CombOffsetFlag));
    jsonRtn->Set(String::NewFromUtf8(isolate,"CombHedgeFlag"), String::NewFromUtf8(isolate,pOrder->CombHedgeFlag));
    jsonRtn->Set(String::NewFromUtf8(isolate,"LimitPrice"), Number::New(pOrder->LimitPrice));
    jsonRtn->Set(String::NewFromUtf8(isolate,"VolumeTotalOriginal"), Int32::New(pOrder->VolumeTotalOriginal));
    jsonRtn->Set(String::NewFromUtf8(isolate,"TimeCondition"), String::NewFromUtf8(isolate,charto_string(pOrder->TimeCondition).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"GTDDate"), String::NewFromUtf8(isolate,pOrder->GTDDate));
    jsonRtn->Set(String::NewFromUtf8(isolate,"VolumeCondition"), String::NewFromUtf8(isolate,charto_string(pOrder->VolumeCondition).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"MinVolume"), Int32::New(pOrder->MinVolume));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ContingentCondition"), String::NewFromUtf8(isolate,charto_string(pOrder->ContingentCondition).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"StopPrice"), Number::New(pOrder->StopPrice));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ForceCloseReason"), String::NewFromUtf8(isolate,charto_string(pOrder->ForceCloseReason).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"IsAutoSuspend"), Int32::New(pOrder->IsAutoSuspend));
    jsonRtn->Set(String::NewFromUtf8(isolate,"BusinessUnit"), String::NewFromUtf8(isolate,pOrder->BusinessUnit));
    jsonRtn->Set(String::NewFromUtf8(isolate,"RequestID"), Int32::New(pOrder->RequestID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OrderLocalID"), String::NewFromUtf8(isolate,pOrder->OrderLocalID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ExchangeID"), String::NewFromUtf8(isolate,pOrder->ExchangeID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ParticipantID"), String::NewFromUtf8(isolate,pOrder->ParticipantID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ClientID"), String::NewFromUtf8(isolate,pOrder->ClientID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ExchangeInstID"), String::NewFromUtf8(isolate,pOrder->ExchangeInstID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"TraderID"), String::NewFromUtf8(isolate,pOrder->TraderID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"InstallID"), Int32::New(pOrder->InstallID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OrderSubmitStatus"), String::NewFromUtf8(isolate,charto_string(pOrder->OrderSubmitStatus).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"NotifySequence"), Int32::New(pOrder->NotifySequence));
    jsonRtn->Set(String::NewFromUtf8(isolate,"TradingDay"), String::NewFromUtf8(isolate,pOrder->TradingDay));
    jsonRtn->Set(String::NewFromUtf8(isolate,"SettlementID"), Int32::New(pOrder->SettlementID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OrderSysID"), String::NewFromUtf8(isolate,pOrder->OrderSysID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OrderSource"), Int32::New(pOrder->OrderSource));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OrderStatus"), String::NewFromUtf8(isolate,charto_string(pOrder->OrderStatus).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OrderType"), String::NewFromUtf8(isolate,charto_string(pOrder->OrderType).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"VolumeTraded"), Int32::New(pOrder->VolumeTraded));
    jsonRtn->Set(String::NewFromUtf8(isolate,"VolumeTotal"), Int32::New(pOrder->VolumeTotal));
    jsonRtn->Set(String::NewFromUtf8(isolate,"InsertDate"), String::NewFromUtf8(isolate,pOrder->InsertDate));
    jsonRtn->Set(String::NewFromUtf8(isolate,"InsertTime"), String::NewFromUtf8(isolate,pOrder->InsertTime));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ActiveTime"), String::NewFromUtf8(isolate,pOrder->ActiveTime));
    jsonRtn->Set(String::NewFromUtf8(isolate,"SuspendTime"), String::NewFromUtf8(isolate,pOrder->SuspendTime));
    jsonRtn->Set(String::NewFromUtf8(isolate,"UpdateTime"), String::NewFromUtf8(isolate,pOrder->UpdateTime));
    jsonRtn->Set(String::NewFromUtf8(isolate,"CancelTime"), String::NewFromUtf8(isolate,pOrder->CancelTime));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ActiveTraderID"), String::NewFromUtf8(isolate,pOrder->ActiveTraderID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ClearingPartID"), String::NewFromUtf8(isolate,pOrder->ClearingPartID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"SequenceNo"), Int32::New(pOrder->SequenceNo));
    jsonRtn->Set(String::NewFromUtf8(isolate,"FrontID"), Int32::New(pOrder->FrontID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"SessionID"), Int32::New(pOrder->SessionID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"UserProductInfo"), String::NewFromUtf8(isolate,pOrder->UserProductInfo));
    jsonRtn->Set(String::NewFromUtf8(isolate,"StatusMsg"), String::NewFromUtf8(isolate,pOrder->StatusMsg));
    jsonRtn->Set(String::NewFromUtf8(isolate,"UserForceClose"), Int32::New(pOrder->UserForceClose));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ActiveUserID"), String::NewFromUtf8(isolate,pOrder->ActiveUserID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"BrokerOrderSeq"), Int32::New(pOrder->BrokerOrderSeq));
    jsonRtn->Set(String::NewFromUtf8(isolate,"RelativeOrderSysID"), String::NewFromUtf8(isolate,pOrder->RelativeOrderSysID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ZCETotalTradedVolume"), Int32::New(pOrder->ZCETotalTradedVolume));
    jsonRtn->Set(String::NewFromUtf8(isolate,"IsSwapOrder"), Int32::New(pOrder->IsSwapOrder));
    *(cbArray + 2) = jsonRtn;
    }
    else {
    *(cbArray + 2) = Local<Value>::New(Undefined(isolate));
    }
    *(cbArray + 3) = pkg_rspinfo(data->rspInfo);
    return;
}
void WrapTrader::pkg_cb_rtnorder(CbRtnField* data, Local<Value>*cbArray) {
    if (data->rtnField){ 
        CThostFtdcOrderField* pOrder = static_cast<CThostFtdcOrderField*>(data->rtnField);
    Local<Object> jsonRtn = Object::New();
    jsonRtn->Set(String::NewFromUtf8(isolate,"BrokerID"), String::NewFromUtf8(isolate,pOrder->BrokerID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"InvestorID"), String::NewFromUtf8(isolate,pOrder->InvestorID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"InstrumentID"), String::NewFromUtf8(isolate,pOrder->InstrumentID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OrderRef"), String::NewFromUtf8(isolate,pOrder->OrderRef));
    jsonRtn->Set(String::NewFromUtf8(isolate,"UserID"), String::NewFromUtf8(isolate,pOrder->UserID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OrderPriceType"), String::NewFromUtf8(isolate,charto_string(pOrder->OrderPriceType).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"Direction"), String::NewFromUtf8(isolate,charto_string(pOrder->Direction).c_str()));  //var charval = String.fromCharCode(asciival);
    jsonRtn->Set(String::NewFromUtf8(isolate,"CombOffsetFlag"), String::NewFromUtf8(isolate,pOrder->CombOffsetFlag));
    jsonRtn->Set(String::NewFromUtf8(isolate,"CombHedgeFlag"), String::NewFromUtf8(isolate,pOrder->CombHedgeFlag));
    jsonRtn->Set(String::NewFromUtf8(isolate,"LimitPrice"), Number::New(pOrder->LimitPrice));
    jsonRtn->Set(String::NewFromUtf8(isolate,"VolumeTotalOriginal"), Int32::New(pOrder->VolumeTotalOriginal));
    jsonRtn->Set(String::NewFromUtf8(isolate,"TimeCondition"), String::NewFromUtf8(isolate,charto_string(pOrder->TimeCondition).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"GTDDate"), String::NewFromUtf8(isolate,pOrder->GTDDate));
    jsonRtn->Set(String::NewFromUtf8(isolate,"VolumeCondition"), String::NewFromUtf8(isolate,charto_string(pOrder->VolumeCondition).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"MinVolume"), Int32::New(pOrder->MinVolume));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ContingentCondition"), String::NewFromUtf8(isolate,charto_string(pOrder->ContingentCondition).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"StopPrice"), Number::New(pOrder->StopPrice));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ForceCloseReason"), String::NewFromUtf8(isolate,charto_string(pOrder->ForceCloseReason).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"IsAutoSuspend"), Int32::New(pOrder->IsAutoSuspend));
    jsonRtn->Set(String::NewFromUtf8(isolate,"BusinessUnit"), String::NewFromUtf8(isolate,pOrder->BusinessUnit));
    jsonRtn->Set(String::NewFromUtf8(isolate,"RequestID"), Int32::New(pOrder->RequestID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OrderLocalID"), String::NewFromUtf8(isolate,pOrder->OrderLocalID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ExchangeID"), String::NewFromUtf8(isolate,pOrder->ExchangeID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ParticipantID"), String::NewFromUtf8(isolate,pOrder->ParticipantID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ClientID"), String::NewFromUtf8(isolate,pOrder->ClientID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ExchangeInstID"), String::NewFromUtf8(isolate,pOrder->ExchangeInstID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"TraderID"), String::NewFromUtf8(isolate,pOrder->TraderID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"InstallID"), Int32::New(pOrder->InstallID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OrderSubmitStatus"), String::NewFromUtf8(isolate,charto_string(pOrder->OrderSubmitStatus).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"NotifySequence"), Int32::New(pOrder->NotifySequence));
    jsonRtn->Set(String::NewFromUtf8(isolate,"TradingDay"), String::NewFromUtf8(isolate,pOrder->TradingDay));
    jsonRtn->Set(String::NewFromUtf8(isolate,"SettlementID"), Int32::New(pOrder->SettlementID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OrderSysID"), String::NewFromUtf8(isolate,pOrder->OrderSysID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OrderSource"), Int32::New(pOrder->OrderSource));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OrderStatus"), String::NewFromUtf8(isolate,charto_string(pOrder->OrderStatus).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OrderType"), String::NewFromUtf8(isolate,charto_string(pOrder->OrderType).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"VolumeTraded"), Int32::New(pOrder->VolumeTraded));
    jsonRtn->Set(String::NewFromUtf8(isolate,"VolumeTotal"), Int32::New(pOrder->VolumeTotal));
    jsonRtn->Set(String::NewFromUtf8(isolate,"InsertDate"), String::NewFromUtf8(isolate,pOrder->InsertDate));
    jsonRtn->Set(String::NewFromUtf8(isolate,"InsertTime"), String::NewFromUtf8(isolate,pOrder->InsertTime));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ActiveTime"), String::NewFromUtf8(isolate,pOrder->ActiveTime));
    jsonRtn->Set(String::NewFromUtf8(isolate,"SuspendTime"), String::NewFromUtf8(isolate,pOrder->SuspendTime));
    jsonRtn->Set(String::NewFromUtf8(isolate,"UpdateTime"), String::NewFromUtf8(isolate,pOrder->UpdateTime));
    jsonRtn->Set(String::NewFromUtf8(isolate,"CancelTime"), String::NewFromUtf8(isolate,pOrder->CancelTime));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ActiveTraderID"), String::NewFromUtf8(isolate,pOrder->ActiveTraderID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ClearingPartID"), String::NewFromUtf8(isolate,pOrder->ClearingPartID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"SequenceNo"), Int32::New(pOrder->SequenceNo));
    jsonRtn->Set(String::NewFromUtf8(isolate,"FrontID"), Int32::New(pOrder->FrontID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"SessionID"), Int32::New(pOrder->SessionID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"UserProductInfo"), String::NewFromUtf8(isolate,pOrder->UserProductInfo));
    jsonRtn->Set(String::NewFromUtf8(isolate,"StatusMsg"), String::NewFromUtf8(isolate,pOrder->StatusMsg));
    jsonRtn->Set(String::NewFromUtf8(isolate,"UserForceClose"), Int32::New(pOrder->UserForceClose));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ActiveUserID"), String::NewFromUtf8(isolate,pOrder->ActiveUserID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"BrokerOrderSeq"), Int32::New(pOrder->BrokerOrderSeq));
    jsonRtn->Set(String::NewFromUtf8(isolate,"RelativeOrderSysID"), String::NewFromUtf8(isolate,pOrder->RelativeOrderSysID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ZCETotalTradedVolume"), Int32::New(pOrder->ZCETotalTradedVolume));
    jsonRtn->Set(String::NewFromUtf8(isolate,"IsSwapOrder"), Int32::New(pOrder->IsSwapOrder));
    *cbArray = jsonRtn;
    }
    else {
    *cbArray = Local<Value>::New(Undefined(isolate));
    }
    return;
}
void WrapTrader::pkg_cb_rqtrade(CbRtnField* data, Local<Value>*cbArray) {
    *cbArray = Int32::New(data->nRequestID);
    *(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
    if (data->rtnField){ 
        CThostFtdcTradeField* pTrade = static_cast<CThostFtdcTradeField*>(data->rtnField);
    Local<Object> jsonRtn = Object::New();
    jsonRtn->Set(String::NewFromUtf8(isolate,"BrokerID"), String::NewFromUtf8(isolate,pTrade->BrokerID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"InvestorID"), String::NewFromUtf8(isolate,pTrade->InvestorID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"InstrumentID"), String::NewFromUtf8(isolate,pTrade->InstrumentID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OrderRef"), String::NewFromUtf8(isolate,pTrade->OrderRef));
    jsonRtn->Set(String::NewFromUtf8(isolate,"UserID"), String::NewFromUtf8(isolate,pTrade->UserID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ExchangeID"), String::NewFromUtf8(isolate,pTrade->ExchangeID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"TradeID"), String::NewFromUtf8(isolate,pTrade->TradeID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"Direction"), String::NewFromUtf8(isolate,charto_string(pTrade->Direction).c_str()));  //var charval = String.fromCharCode(asciival);
    jsonRtn->Set(String::NewFromUtf8(isolate,"OrderSysID"), String::NewFromUtf8(isolate,pTrade->OrderSysID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ParticipantID"), String::NewFromUtf8(isolate,pTrade->ParticipantID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ClientID"), String::NewFromUtf8(isolate,pTrade->ClientID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"TradingRole"), String::NewFromUtf8(isolate,charto_string(pTrade->TradingRole).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ExchangeInstID"), String::NewFromUtf8(isolate,pTrade->ExchangeInstID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OffsetFlag"), String::NewFromUtf8(isolate,charto_string(pTrade->OffsetFlag).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"HedgeFlag"), String::NewFromUtf8(isolate,charto_string(pTrade->HedgeFlag).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"Price"), Number::New(pTrade->Price));
    jsonRtn->Set(String::NewFromUtf8(isolate,"Volume"), Int32::New(pTrade->Volume));
    jsonRtn->Set(String::NewFromUtf8(isolate,"TradeDate"), String::NewFromUtf8(isolate,pTrade->TradeDate));
    jsonRtn->Set(String::NewFromUtf8(isolate,"TradeTime"), String::NewFromUtf8(isolate,pTrade->TradeTime));
    jsonRtn->Set(String::NewFromUtf8(isolate,"TradeType"), String::NewFromUtf8(isolate,charto_string(pTrade->TradeType).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"PriceSource"), String::NewFromUtf8(isolate,charto_string(pTrade->PriceSource).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"TraderID"), String::NewFromUtf8(isolate,pTrade->TraderID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OrderLocalID"), String::NewFromUtf8(isolate,pTrade->OrderLocalID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ClearingPartID"), String::NewFromUtf8(isolate,pTrade->ClearingPartID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"BusinessUnit"), String::NewFromUtf8(isolate,pTrade->BusinessUnit));
    jsonRtn->Set(String::NewFromUtf8(isolate,"SequenceNo"), Int32::New(pTrade->SequenceNo));
    jsonRtn->Set(String::NewFromUtf8(isolate,"TradingDay"), String::NewFromUtf8(isolate,pTrade->TradingDay));
    jsonRtn->Set(String::NewFromUtf8(isolate,"SettlementID"), Int32::New(pTrade->SettlementID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"BrokerOrderSeq"), Int32::New(pTrade->BrokerOrderSeq));
    jsonRtn->Set(String::NewFromUtf8(isolate,"TradeSource"), String::NewFromUtf8(isolate,charto_string(pTrade->TradeSource).c_str()));
    *(cbArray + 2) = jsonRtn;
    }
    else {
    *(cbArray + 2) = Local<Value>::New(Undefined(isolate));
    }
    *(cbArray + 3) = pkg_rspinfo(data->rspInfo);
    return;
}
void WrapTrader::pkg_cb_rtntrade(CbRtnField* data, Local<Value>*cbArray) {
    if (data->rtnField){ 
        CThostFtdcTradeField* pTrade = static_cast<CThostFtdcTradeField*>(data->rtnField);
    Local<Object> jsonRtn = Object::New();
    jsonRtn->Set(String::NewFromUtf8(isolate,"BrokerID"), String::NewFromUtf8(isolate,pTrade->BrokerID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"InvestorID"), String::NewFromUtf8(isolate,pTrade->InvestorID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"InstrumentID"), String::NewFromUtf8(isolate,pTrade->InstrumentID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OrderRef"), String::NewFromUtf8(isolate,pTrade->OrderRef));
    jsonRtn->Set(String::NewFromUtf8(isolate,"UserID"), String::NewFromUtf8(isolate,pTrade->UserID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ExchangeID"), String::NewFromUtf8(isolate,pTrade->ExchangeID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"TradeID"), String::NewFromUtf8(isolate,pTrade->TradeID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"Direction"), String::NewFromUtf8(isolate,charto_string(pTrade->Direction).c_str()));  //var charval = String.fromCharCode(asciival);
    jsonRtn->Set(String::NewFromUtf8(isolate,"OrderSysID"), String::NewFromUtf8(isolate,pTrade->OrderSysID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ParticipantID"), String::NewFromUtf8(isolate,pTrade->ParticipantID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ClientID"), String::NewFromUtf8(isolate,pTrade->ClientID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"TradingRole"), Int32::New(pTrade->TradingRole));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ExchangeInstID"), String::NewFromUtf8(isolate,pTrade->ExchangeInstID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OffsetFlag"), Int32::New(pTrade->OffsetFlag));
    jsonRtn->Set(String::NewFromUtf8(isolate,"HedgeFlag"), Int32::New(pTrade->HedgeFlag));
    jsonRtn->Set(String::NewFromUtf8(isolate,"Price"), Number::New(pTrade->Price));
    jsonRtn->Set(String::NewFromUtf8(isolate,"Volume"), Int32::New(pTrade->Volume));
    jsonRtn->Set(String::NewFromUtf8(isolate,"TradeDate"), String::NewFromUtf8(isolate,pTrade->TradeDate));
    jsonRtn->Set(String::NewFromUtf8(isolate,"TradeTime"), String::NewFromUtf8(isolate,pTrade->TradeTime));
    jsonRtn->Set(String::NewFromUtf8(isolate,"TradeType"), Int32::New(pTrade->TradeType));
    jsonRtn->Set(String::NewFromUtf8(isolate,"PriceSource"), String::NewFromUtf8(isolate,charto_string(pTrade->PriceSource).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"TraderID"), String::NewFromUtf8(isolate,pTrade->TraderID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OrderLocalID"), String::NewFromUtf8(isolate,pTrade->OrderLocalID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ClearingPartID"), String::NewFromUtf8(isolate,pTrade->ClearingPartID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"BusinessUnit"), String::NewFromUtf8(isolate,pTrade->BusinessUnit));
    jsonRtn->Set(String::NewFromUtf8(isolate,"SequenceNo"), Int32::New(pTrade->SequenceNo));
    jsonRtn->Set(String::NewFromUtf8(isolate,"TradingDay"), String::NewFromUtf8(isolate,pTrade->TradingDay));
    jsonRtn->Set(String::NewFromUtf8(isolate,"SettlementID"), Int32::New(pTrade->SettlementID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"BrokerOrderSeq"), Int32::New(pTrade->BrokerOrderSeq));
    jsonRtn->Set(String::NewFromUtf8(isolate,"TradeSource"), String::NewFromUtf8(isolate,charto_string(pTrade->TradeSource).c_str()));
    *cbArray = jsonRtn;
    }
    else {
    *cbArray = Local<Value>::New(Undefined(isolate));
    }
     
    return;
}
void WrapTrader::pkg_cb_rqinvestorposition(CbRtnField* data, Local<Value>*cbArray) {
    *cbArray = Int32::New(data->nRequestID);
    *(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
    if (data->rtnField){ 
        CThostFtdcInvestorPositionField* _pInvestorPosition = static_cast<CThostFtdcInvestorPositionField*>(data->rtnField);
    Local<Object> jsonRtn = Object::New();
    jsonRtn->Set(String::NewFromUtf8(isolate,"InstrumentID"), String::NewFromUtf8(isolate,_pInvestorPosition->InstrumentID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"BrokerID"), String::NewFromUtf8(isolate,_pInvestorPosition->BrokerID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"InvestorID"), String::NewFromUtf8(isolate,_pInvestorPosition->InvestorID)); 
    jsonRtn->Set(String::NewFromUtf8(isolate,"PosiDirection"), String::NewFromUtf8(isolate,charto_string(_pInvestorPosition->PosiDirection).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"HedgeFlag"), String::NewFromUtf8(isolate,charto_string(_pInvestorPosition->HedgeFlag).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"PositionDate"), Int32::New(_pInvestorPosition->PositionDate));
    jsonRtn->Set(String::NewFromUtf8(isolate,"YdPosition"), Int32::New(_pInvestorPosition->YdPosition));
    jsonRtn->Set(String::NewFromUtf8(isolate,"Position"), Int32::New(_pInvestorPosition->Position));
    jsonRtn->Set(String::NewFromUtf8(isolate,"LongFrozen"), Int32::New(_pInvestorPosition->LongFrozen));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ShortFrozen"), Int32::New(_pInvestorPosition->ShortFrozen));
    jsonRtn->Set(String::NewFromUtf8(isolate,"LongFrozenAmount"), Number::New(_pInvestorPosition->LongFrozenAmount));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ShortFrozenAmount"), Number::New(_pInvestorPosition->ShortFrozenAmount));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OpenVolume"), Int32::New(_pInvestorPosition->OpenVolume));
    jsonRtn->Set(String::NewFromUtf8(isolate,"CloseVolume"), Int32::New(_pInvestorPosition->CloseVolume));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OpenAmount"), Number::New(_pInvestorPosition->OpenAmount));
    jsonRtn->Set(String::NewFromUtf8(isolate,"CloseAmount"), Number::New(_pInvestorPosition->CloseAmount));
    jsonRtn->Set(String::NewFromUtf8(isolate,"PositionCost"), Number::New(_pInvestorPosition->PositionCost));
    jsonRtn->Set(String::NewFromUtf8(isolate,"PreMargin"), Number::New(_pInvestorPosition->PreMargin));
    jsonRtn->Set(String::NewFromUtf8(isolate,"UseMargin"), Number::New(_pInvestorPosition->UseMargin));
    jsonRtn->Set(String::NewFromUtf8(isolate,"FrozenMargin"), Number::New(_pInvestorPosition->FrozenMargin));
    jsonRtn->Set(String::NewFromUtf8(isolate,"FrozenCash"), Number::New(_pInvestorPosition->FrozenCash));
    jsonRtn->Set(String::NewFromUtf8(isolate,"FrozenCommission"), Number::New(_pInvestorPosition->FrozenCommission));
    jsonRtn->Set(String::NewFromUtf8(isolate,"CashIn"), Number::New(_pInvestorPosition->CashIn));
    jsonRtn->Set(String::NewFromUtf8(isolate,"Commission"), Number::New(_pInvestorPosition->Commission));
    jsonRtn->Set(String::NewFromUtf8(isolate,"CloseProfit"), Number::New(_pInvestorPosition->CloseProfit));
    jsonRtn->Set(String::NewFromUtf8(isolate,"PositionProfit"), Number::New(_pInvestorPosition->PositionProfit));
    jsonRtn->Set(String::NewFromUtf8(isolate,"PreSettlementPrice"), Number::New(_pInvestorPosition->PreSettlementPrice));
    jsonRtn->Set(String::NewFromUtf8(isolate,"SettlementPrice"), Number::New(_pInvestorPosition->SettlementPrice));
    jsonRtn->Set(String::NewFromUtf8(isolate,"TradingDay"), String::NewFromUtf8(isolate,_pInvestorPosition->TradingDay));
    jsonRtn->Set(String::NewFromUtf8(isolate,"SettlementID"), Int32::New(_pInvestorPosition->SettlementID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OpenCost"), Number::New(_pInvestorPosition->OpenCost));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ExchangeMargin"), Number::New(_pInvestorPosition->ExchangeMargin));
    jsonRtn->Set(String::NewFromUtf8(isolate,"CombPosition"), Int32::New(_pInvestorPosition->CombPosition));
    jsonRtn->Set(String::NewFromUtf8(isolate,"CombLongFrozen"), Int32::New(_pInvestorPosition->CombLongFrozen));
    jsonRtn->Set(String::NewFromUtf8(isolate,"CombShortFrozen"), Int32::New(_pInvestorPosition->CombShortFrozen));
    jsonRtn->Set(String::NewFromUtf8(isolate,"CloseProfitByDate"), Number::New(_pInvestorPosition->CloseProfitByDate));
    jsonRtn->Set(String::NewFromUtf8(isolate,"CloseProfitByTrade"), Number::New(_pInvestorPosition->CloseProfitByTrade));
    jsonRtn->Set(String::NewFromUtf8(isolate,"TodayPosition"), Int32::New(_pInvestorPosition->TodayPosition));
    jsonRtn->Set(String::NewFromUtf8(isolate,"MarginRateByMoney"), Number::New(_pInvestorPosition->MarginRateByMoney));
    jsonRtn->Set(String::NewFromUtf8(isolate,"MarginRateByVolume"), Number::New(_pInvestorPosition->MarginRateByVolume));
    *(cbArray + 2) = jsonRtn;
    }
    else {
    *(cbArray + 2) = Local<Value>::New(Undefined(isolate));
    }
    *(cbArray + 3) = pkg_rspinfo(data->rspInfo);
    return;
}
void WrapTrader::pkg_cb_rqinvestorpositiondetail(CbRtnField* data, Local<Value>*cbArray) {
    *cbArray = Int32::New(data->nRequestID);
    *(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
    if (data->rtnField){ 
        CThostFtdcInvestorPositionDetailField* pInvestorPositionDetail = static_cast<CThostFtdcInvestorPositionDetailField*>(data->rtnField);
    Local<Object> jsonRtn = Object::New();
    jsonRtn->Set(String::NewFromUtf8(isolate,"InstrumentID"), String::NewFromUtf8(isolate,pInvestorPositionDetail->InstrumentID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"BrokerID"), String::NewFromUtf8(isolate,pInvestorPositionDetail->BrokerID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"InvestorID"), String::NewFromUtf8(isolate,pInvestorPositionDetail->InvestorID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"HedgeFlag"), String::NewFromUtf8(isolate,charto_string(pInvestorPositionDetail->HedgeFlag).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"Direction"), String::NewFromUtf8(isolate,charto_string(pInvestorPositionDetail->Direction).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OpenDate"), String::NewFromUtf8(isolate,pInvestorPositionDetail->OpenDate));
    jsonRtn->Set(String::NewFromUtf8(isolate,"TradeID"), String::NewFromUtf8(isolate,pInvestorPositionDetail->TradeID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"Volume"), Int32::New(pInvestorPositionDetail->Volume));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OpenPrice"), Number::New(pInvestorPositionDetail->OpenPrice));
    jsonRtn->Set(String::NewFromUtf8(isolate,"TradingDay"), String::NewFromUtf8(isolate,pInvestorPositionDetail->TradingDay));
    jsonRtn->Set(String::NewFromUtf8(isolate,"SettlementID"), Int32::New(pInvestorPositionDetail->SettlementID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"TradeType"), String::NewFromUtf8(isolate,charto_string(pInvestorPositionDetail->TradeType).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"CombInstrumentID"), String::NewFromUtf8(isolate,pInvestorPositionDetail->CombInstrumentID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ExchangeID"), String::NewFromUtf8(isolate,pInvestorPositionDetail->ExchangeID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"CloseProfitByDate"), Number::New(pInvestorPositionDetail->CloseProfitByDate));
    jsonRtn->Set(String::NewFromUtf8(isolate,"CloseProfitByTrade"), Number::New(pInvestorPositionDetail->CloseProfitByTrade));
    jsonRtn->Set(String::NewFromUtf8(isolate,"PositionProfitByDate"), Number::New(pInvestorPositionDetail->PositionProfitByDate));
    jsonRtn->Set(String::NewFromUtf8(isolate,"PositionProfitByTrade"), Number::New(pInvestorPositionDetail->PositionProfitByTrade));
    jsonRtn->Set(String::NewFromUtf8(isolate,"Margin"), Number::New(pInvestorPositionDetail->Margin));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ExchMargin"), Number::New(pInvestorPositionDetail->ExchMargin));
    jsonRtn->Set(String::NewFromUtf8(isolate,"MarginRateByMoney"), Number::New(pInvestorPositionDetail->MarginRateByMoney));
    jsonRtn->Set(String::NewFromUtf8(isolate,"MarginRateByVolume"), Number::New(pInvestorPositionDetail->MarginRateByVolume));
    jsonRtn->Set(String::NewFromUtf8(isolate,"LastSettlementPrice"), Number::New(pInvestorPositionDetail->LastSettlementPrice));
    jsonRtn->Set(String::NewFromUtf8(isolate,"SettlementPrice"), Number::New(pInvestorPositionDetail->SettlementPrice));
    jsonRtn->Set(String::NewFromUtf8(isolate,"CloseVolume"), Int32::New(pInvestorPositionDetail->CloseVolume));
    jsonRtn->Set(String::NewFromUtf8(isolate,"CloseAmount"), Number::New(pInvestorPositionDetail->CloseAmount));
    *(cbArray + 2) = jsonRtn;
    }
    else {
    *(cbArray + 2) = Local<Value>::New(Undefined(isolate));
    }
    *(cbArray + 3) = pkg_rspinfo(data->rspInfo);
    return;
}
void WrapTrader::pkg_cb_rqtradingaccount(CbRtnField* data, Local<Value>*cbArray) {
    *cbArray = Int32::New(data->nRequestID);
    *(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
    if (data->rtnField){ 
        CThostFtdcTradingAccountField *pTradingAccount = static_cast<CThostFtdcTradingAccountField*>(data->rtnField);
    Local<Object> jsonRtn = Object::New();
    jsonRtn->Set(String::NewFromUtf8(isolate,"BrokerID"), String::NewFromUtf8(isolate,pTradingAccount->BrokerID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"AccountID"), String::NewFromUtf8(isolate,pTradingAccount->AccountID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"PreMortgage"), Number::New(pTradingAccount->PreMortgage));
    jsonRtn->Set(String::NewFromUtf8(isolate,"PreCredit"), Number::New(pTradingAccount->PreCredit));
    jsonRtn->Set(String::NewFromUtf8(isolate,"PreDeposit"), Number::New(pTradingAccount->PreDeposit));
    jsonRtn->Set(String::NewFromUtf8(isolate,"PreBalance"), Number::New(pTradingAccount->PreBalance));
    jsonRtn->Set(String::NewFromUtf8(isolate,"PreMargin"), Number::New(pTradingAccount->PreMargin));
    jsonRtn->Set(String::NewFromUtf8(isolate,"InterestBase"), Number::New(pTradingAccount->InterestBase));
    jsonRtn->Set(String::NewFromUtf8(isolate,"Interest"), Number::New(pTradingAccount->Interest));
    jsonRtn->Set(String::NewFromUtf8(isolate,"Deposit"), Number::New(pTradingAccount->Deposit));
    jsonRtn->Set(String::NewFromUtf8(isolate,"Withdraw"), Number::New(pTradingAccount->Withdraw));
    jsonRtn->Set(String::NewFromUtf8(isolate,"FrozenMargin"), Number::New(pTradingAccount->FrozenMargin));
    jsonRtn->Set(String::NewFromUtf8(isolate,"FrozenCash"), Number::New(pTradingAccount->FrozenCash));
    jsonRtn->Set(String::NewFromUtf8(isolate,"FrozenCommission"), Number::New(pTradingAccount->FrozenCommission));
    jsonRtn->Set(String::NewFromUtf8(isolate,"CurrMargin"), Number::New(pTradingAccount->CurrMargin));
    jsonRtn->Set(String::NewFromUtf8(isolate,"CashIn"), Number::New(pTradingAccount->CashIn));
    jsonRtn->Set(String::NewFromUtf8(isolate,"Commission"), Number::New(pTradingAccount->Commission));
    jsonRtn->Set(String::NewFromUtf8(isolate,"CloseProfit"), Number::New(pTradingAccount->CloseProfit));
    jsonRtn->Set(String::NewFromUtf8(isolate,"PositionProfit"), Number::New(pTradingAccount->PositionProfit));
    jsonRtn->Set(String::NewFromUtf8(isolate,"Balance"), Number::New(pTradingAccount->Balance));
    jsonRtn->Set(String::NewFromUtf8(isolate,"Available"), Number::New(pTradingAccount->Available));
    jsonRtn->Set(String::NewFromUtf8(isolate,"WithdrawQuota"), Number::New(pTradingAccount->WithdrawQuota));
    jsonRtn->Set(String::NewFromUtf8(isolate,"Reserve"), Number::New(pTradingAccount->Reserve));
    jsonRtn->Set(String::NewFromUtf8(isolate,"TradingDay"), String::NewFromUtf8(isolate,pTradingAccount->TradingDay));
    jsonRtn->Set(String::NewFromUtf8(isolate,"SettlementID"), Int32::New(pTradingAccount->SettlementID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"Credit"), Number::New(pTradingAccount->Credit));
    jsonRtn->Set(String::NewFromUtf8(isolate,"Mortgage"), Number::New(pTradingAccount->Mortgage));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ExchangeMargin"), Number::New(pTradingAccount->ExchangeMargin));
    jsonRtn->Set(String::NewFromUtf8(isolate,"DeliveryMargin"), Number::New(pTradingAccount->DeliveryMargin));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ExchangeDeliveryMargin"), Number::New(pTradingAccount->ExchangeDeliveryMargin));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ReserveBalance"), Number::New(pTradingAccount->ReserveBalance));
    jsonRtn->Set(String::NewFromUtf8(isolate,"CurrencyID"), String::NewFromUtf8(isolate,pTradingAccount->CurrencyID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"PreFundMortgageIn"), Number::New(pTradingAccount->PreFundMortgageIn));
    jsonRtn->Set(String::NewFromUtf8(isolate,"PreFundMortgageOut"), Number::New(pTradingAccount->PreFundMortgageOut));
    jsonRtn->Set(String::NewFromUtf8(isolate,"FundMortgageIn"), Number::New(pTradingAccount->FundMortgageIn));
    jsonRtn->Set(String::NewFromUtf8(isolate,"FundMortgageOut"), Number::New(pTradingAccount->FundMortgageOut));
    jsonRtn->Set(String::NewFromUtf8(isolate,"FundMortgageAvailable"), Number::New(pTradingAccount->FundMortgageAvailable));
    jsonRtn->Set(String::NewFromUtf8(isolate,"MortgageableFund"), Number::New(pTradingAccount->MortgageableFund));
    jsonRtn->Set(String::NewFromUtf8(isolate,"SpecProductMargin"), Number::New(pTradingAccount->SpecProductMargin));
    jsonRtn->Set(String::NewFromUtf8(isolate,"SpecProductFrozenMargin"), Number::New(pTradingAccount->SpecProductFrozenMargin));
    jsonRtn->Set(String::NewFromUtf8(isolate,"SpecProductCommission"), Number::New(pTradingAccount->SpecProductCommission));
    jsonRtn->Set(String::NewFromUtf8(isolate,"SpecProductFrozenCommission"), Number::New(pTradingAccount->SpecProductFrozenCommission));
    jsonRtn->Set(String::NewFromUtf8(isolate,"SpecProductPositionProfit"), Number::New(pTradingAccount->SpecProductPositionProfit));
    jsonRtn->Set(String::NewFromUtf8(isolate,"SpecProductCloseProfit"), Number::New(pTradingAccount->SpecProductCloseProfit));
    jsonRtn->Set(String::NewFromUtf8(isolate,"SpecProductPositionProfitByAlg"), Number::New(pTradingAccount->SpecProductPositionProfitByAlg));
    jsonRtn->Set(String::NewFromUtf8(isolate,"SpecProductExchangeMargin"), Number::New(pTradingAccount->SpecProductExchangeMargin));
    *(cbArray + 2) = jsonRtn;
    }
    else {
    *(cbArray + 2) = Local<Value>::New(Undefined(isolate));
    }
    *(cbArray + 3) = pkg_rspinfo(data->rspInfo);
    return;
}
void WrapTrader::pkg_cb_rqinstrument(CbRtnField* data, Local<Value>*cbArray) {
    *cbArray = Int32::New(data->nRequestID);
    *(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
    if (data->rtnField){ 
        CThostFtdcInstrumentField *pInstrument = static_cast<CThostFtdcInstrumentField*>(data->rtnField);
    Local<Object> jsonRtn = Object::New();
    jsonRtn->Set(String::NewFromUtf8(isolate,"InstrumentID"), String::NewFromUtf8(isolate,pInstrument->InstrumentID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ExchangeID"), String::NewFromUtf8(isolate,pInstrument->ExchangeID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"InstrumentName"), String::NewFromUtf8(isolate,pInstrument->InstrumentName));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ExchangeInstID"), String::NewFromUtf8(isolate,pInstrument->ExchangeInstID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ProductID"), String::NewFromUtf8(isolate,pInstrument->ProductID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ProductClass"), String::NewFromUtf8(isolate,charto_string(pInstrument->ProductClass).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"DeliveryYear"), Int32::New(pInstrument->DeliveryYear));
    jsonRtn->Set(String::NewFromUtf8(isolate,"DeliveryMonth"), Int32::New(pInstrument->DeliveryMonth));
    jsonRtn->Set(String::NewFromUtf8(isolate,"MaxMarketOrderVolume"), Int32::New(pInstrument->MaxMarketOrderVolume));
    jsonRtn->Set(String::NewFromUtf8(isolate,"MinMarketOrderVolume"), Int32::New(pInstrument->MinMarketOrderVolume));
    jsonRtn->Set(String::NewFromUtf8(isolate,"MaxLimitOrderVolume"), Int32::New(pInstrument->MaxLimitOrderVolume));
    jsonRtn->Set(String::NewFromUtf8(isolate,"MinLimitOrderVolume"), Int32::New(pInstrument->MinLimitOrderVolume));
    jsonRtn->Set(String::NewFromUtf8(isolate,"VolumeMultiple"), Int32::New(pInstrument->VolumeMultiple));
    jsonRtn->Set(String::NewFromUtf8(isolate,"PriceTick"), Number::New(pInstrument->PriceTick));
    jsonRtn->Set(String::NewFromUtf8(isolate,"CreateDate"), String::NewFromUtf8(isolate,pInstrument->CreateDate));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OpenDate"), String::NewFromUtf8(isolate,pInstrument->OpenDate));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ExpireDate"), String::NewFromUtf8(isolate,pInstrument->ExpireDate));
    jsonRtn->Set(String::NewFromUtf8(isolate,"StartDelivDate"), String::NewFromUtf8(isolate,pInstrument->StartDelivDate));
    jsonRtn->Set(String::NewFromUtf8(isolate,"EndDelivDate"), String::NewFromUtf8(isolate,pInstrument->EndDelivDate));
    jsonRtn->Set(String::NewFromUtf8(isolate,"InstLifePhase"), Int32::New(pInstrument->InstLifePhase));
    jsonRtn->Set(String::NewFromUtf8(isolate,"IsTrading"), Int32::New(pInstrument->IsTrading));
    jsonRtn->Set(String::NewFromUtf8(isolate,"PositionType"), String::NewFromUtf8(isolate,charto_string(pInstrument->PositionType).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"PositionDateType"), String::NewFromUtf8(isolate,charto_string(pInstrument->PositionDateType).c_str()));
    jsonRtn->Set(String::NewFromUtf8(isolate,"LongMarginRatio"), Number::New(pInstrument->LongMarginRatio));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ShortMarginRatio"), Number::New(pInstrument->ShortMarginRatio));
    jsonRtn->Set(String::NewFromUtf8(isolate,"MaxMarginSideAlgorithm"), String::NewFromUtf8(isolate,charto_string(pInstrument->MaxMarginSideAlgorithm).c_str()));
    *(cbArray + 2) = jsonRtn;
    }
    else {
    *(cbArray + 2) = Local<Value>::New(Undefined(isolate));
    }
    *(cbArray + 3) = pkg_rspinfo(data->rspInfo);
    return;
}
void WrapTrader::pkg_cb_rqdepthmarketdata(CbRtnField* data, Local<Value>*cbArray) {
    *cbArray = Int32::New(data->nRequestID);
    *(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
    if (data->rtnField){ 
        CThostFtdcDepthMarketDataField *pDepthMarketData = static_cast<CThostFtdcDepthMarketDataField*>(data->rtnField);
    Local<Object> jsonRtn = Object::New();
    jsonRtn->Set(String::NewFromUtf8(isolate,"TradingDay"), String::NewFromUtf8(isolate,pDepthMarketData->TradingDay));
    jsonRtn->Set(String::NewFromUtf8(isolate,"InstrumentID"), String::NewFromUtf8(isolate,pDepthMarketData->InstrumentID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ExchangeID"), String::NewFromUtf8(isolate,pDepthMarketData->ExchangeID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ExchangeInstID"), String::NewFromUtf8(isolate,pDepthMarketData->ExchangeInstID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"LastPrice"), Number::New(pDepthMarketData->LastPrice));
    jsonRtn->Set(String::NewFromUtf8(isolate,"PreSettlementPrice"), Number::New(pDepthMarketData->PreSettlementPrice));
    jsonRtn->Set(String::NewFromUtf8(isolate,"PreClosePrice"), Number::New(pDepthMarketData->PreClosePrice));
    jsonRtn->Set(String::NewFromUtf8(isolate,"PreOpenInterest"), Number::New(pDepthMarketData->PreOpenInterest));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OpenPrice"), Number::New(pDepthMarketData->OpenPrice));
    jsonRtn->Set(String::NewFromUtf8(isolate,"HighestPrice"), Number::New(pDepthMarketData->HighestPrice));
    jsonRtn->Set(String::NewFromUtf8(isolate,"LowestPrice"), Number::New(pDepthMarketData->LowestPrice));
    jsonRtn->Set(String::NewFromUtf8(isolate,"Volume"), Int32::New(pDepthMarketData->Volume));
    jsonRtn->Set(String::NewFromUtf8(isolate,"Turnover"), Number::New(pDepthMarketData->Turnover));
    jsonRtn->Set(String::NewFromUtf8(isolate,"OpenInterest"), Number::New(pDepthMarketData->OpenInterest));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ClosePrice"), Number::New(pDepthMarketData->ClosePrice));
    jsonRtn->Set(String::NewFromUtf8(isolate,"SettlementPrice"), Number::New(pDepthMarketData->SettlementPrice));
    jsonRtn->Set(String::NewFromUtf8(isolate,"UpperLimitPrice"), Number::New(pDepthMarketData->UpperLimitPrice));
    jsonRtn->Set(String::NewFromUtf8(isolate,"LowerLimitPrice"), Number::New(pDepthMarketData->LowerLimitPrice));
    jsonRtn->Set(String::NewFromUtf8(isolate,"PreDelta"), Number::New(pDepthMarketData->PreDelta));
    jsonRtn->Set(String::NewFromUtf8(isolate,"CurrDelta"), Number::New(pDepthMarketData->CurrDelta));
    jsonRtn->Set(String::NewFromUtf8(isolate,"UpdateTime"), String::NewFromUtf8(isolate,pDepthMarketData->UpdateTime));
    jsonRtn->Set(String::NewFromUtf8(isolate,"UpdateMillisec"), Int32::New(pDepthMarketData->UpdateMillisec));
    jsonRtn->Set(String::NewFromUtf8(isolate,"BidPrice1"), Number::New(pDepthMarketData->BidPrice1));
    jsonRtn->Set(String::NewFromUtf8(isolate,"BidVolume1"), Number::New(pDepthMarketData->BidVolume1));
    jsonRtn->Set(String::NewFromUtf8(isolate,"AskPrice1"), Number::New(pDepthMarketData->AskPrice1));
    jsonRtn->Set(String::NewFromUtf8(isolate,"AskVolume1"), Number::New(pDepthMarketData->AskVolume1));
    jsonRtn->Set(String::NewFromUtf8(isolate,"BidPrice2"), Number::New(pDepthMarketData->BidPrice2));
    jsonRtn->Set(String::NewFromUtf8(isolate,"BidVolume2"), Number::New(pDepthMarketData->BidVolume2));
    jsonRtn->Set(String::NewFromUtf8(isolate,"AskPrice2"), Number::New(pDepthMarketData->AskPrice2));
    jsonRtn->Set(String::NewFromUtf8(isolate,"AskVolume2"), Number::New(pDepthMarketData->AskVolume2));
    jsonRtn->Set(String::NewFromUtf8(isolate,"BidPrice3"), Number::New(pDepthMarketData->BidPrice3));
    jsonRtn->Set(String::NewFromUtf8(isolate,"BidVolume3"), Number::New(pDepthMarketData->BidVolume3));
    jsonRtn->Set(String::NewFromUtf8(isolate,"AskPrice3"), Number::New(pDepthMarketData->AskPrice3));
    jsonRtn->Set(String::NewFromUtf8(isolate,"AskVolume3"), Number::New(pDepthMarketData->AskVolume3));
    jsonRtn->Set(String::NewFromUtf8(isolate,"BidPrice4"), Number::New(pDepthMarketData->BidPrice4));
    jsonRtn->Set(String::NewFromUtf8(isolate,"BidVolume4"), Number::New(pDepthMarketData->BidVolume4));
    jsonRtn->Set(String::NewFromUtf8(isolate,"AskPrice4"), Number::New(pDepthMarketData->AskPrice4));
    jsonRtn->Set(String::NewFromUtf8(isolate,"AskVolume4"), Number::New(pDepthMarketData->AskVolume4));
    jsonRtn->Set(String::NewFromUtf8(isolate,"BidPrice5"), Number::New(pDepthMarketData->BidPrice5));
    jsonRtn->Set(String::NewFromUtf8(isolate,"BidVolume5"), Number::New(pDepthMarketData->BidVolume5));
    jsonRtn->Set(String::NewFromUtf8(isolate,"AskPrice5"), Number::New(pDepthMarketData->AskPrice5));
    jsonRtn->Set(String::NewFromUtf8(isolate,"AskVolume5"), Number::New(pDepthMarketData->AskVolume5));
    jsonRtn->Set(String::NewFromUtf8(isolate,"AveragePrice"), Number::New(pDepthMarketData->AveragePrice));
    jsonRtn->Set(String::NewFromUtf8(isolate,"ActionDay"), String::NewFromUtf8(isolate,pDepthMarketData->ActionDay));
    *(cbArray + 2) = jsonRtn;
    }
    else {
    *(cbArray + 2) = Local<Value>::New(Undefined(isolate));
    }
    *(cbArray + 3) = pkg_rspinfo(data->rspInfo);
    return;
}
void WrapTrader::pkg_cb_rqsettlementinfo(CbRtnField* data, Local<Value>*cbArray) {
    *cbArray = Int32::New(data->nRequestID);
    *(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
    if(data->rtnField!=NULL){
        CThostFtdcSettlementInfoField *pSettlementInfo = static_cast<CThostFtdcSettlementInfoField*>(data->rtnField);
    Local<Object> jsonRtn = Object::New();
    jsonRtn->Set(String::NewFromUtf8(isolate,"TradingDay"), String::NewFromUtf8(isolate,pSettlementInfo->TradingDay));
    jsonRtn->Set(String::NewFromUtf8(isolate,"SettlementID"), Int32::New(pSettlementInfo->SettlementID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"BrokerID"), String::NewFromUtf8(isolate,pSettlementInfo->BrokerID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"InvestorID"), String::NewFromUtf8(isolate,pSettlementInfo->InvestorID));
    jsonRtn->Set(String::NewFromUtf8(isolate,"SequenceNo"), Int32::New(pSettlementInfo->SequenceNo));
        jsonRtn->Set(String::NewFromUtf8(isolate,"Content"), String::NewFromUtf8(isolate,pSettlementInfo->Content));
        *(cbArray + 2) = jsonRtn;
    }
    else {
        *(cbArray + 2) = Local<Value>::New(Undefined(isolate));
    }
    *(cbArray + 3) = pkg_rspinfo(data->rspInfo);
    return;
}
void WrapTrader::pkg_cb_rsperror(CbRtnField* data, Local<Value>*cbArray) {
    *cbArray = Int32::New(data->nRequestID);
    *(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
    *(cbArray + 2) = pkg_rspinfo(data->rspInfo);
    return;
}
Local<Value> WrapTrader::pkg_rspinfo(void *vpRspInfo) {
    if (vpRspInfo) {
        CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField*>(vpRspInfo);
    Local<Object> jsonInfo = Object::New();
    jsonInfo->Set(String::NewFromUtf8(isolate,"ErrorID"), Int32::New(pRspInfo->ErrorID));
    jsonInfo->Set(String::NewFromUtf8(isolate,"ErrorMsg"), String::NewFromUtf8(isolate,pRspInfo->ErrorMsg));
    return jsonInfo;
    }
    else {
    return     Local<Value>::New(Undefined(isolate));
    }
}
