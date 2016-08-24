#include <node.h>
#include "wrap_trader.h"

Persistent <Function> WrapTrader::constructor;
int WrapTrader::s_uuid;
std::map<const char *, int, ptrCmp> WrapTrader::event_map;
//std::map<int, Persistent<Function> > WrapTrader::callback_map(isolate);
//std::map<int, Persistent<Function> > WrapTrader::fun_rtncb_map(isolate);

Isolate *WrapTrader::isolate = NULL;

WrapTrader::WrapTrader() {
    logger_cout("wrap_trader------>object start init");
    uvTrader = new uv_trader();
    logger_cout("wrap_trader------>object init successed");
}

WrapTrader::~WrapTrader(void) {
    if (uvTrader) {
        delete uvTrader;
    }
    logger_cout("wrap_trader------>object destroyed");
}


PersistentValueMap <int, Function, DefaultPersistentValueMapTraits<int, Function>> &WrapTrader::callback_map(
        Isolate * isolate) {
    static PersistentValueMap <int, Function, DefaultPersistentValueMapTraits<int, Function>> &local_ref_map_
            = *new PersistentValueMap <int, Function, DefaultPersistentValueMapTraits<int, Function>>(
                    isolate);
    return local_ref_map_;
}

PersistentValueMap <int, Function, DefaultPersistentValueMapTraits<int, Function>> &WrapTrader::fun_rtncb_map(
        Isolate * isolate) {
    static PersistentValueMap <int, Function, DefaultPersistentValueMapTraits<int, Function>> &local_ref_fun_map_
            = *new PersistentValueMap <int, Function, DefaultPersistentValueMapTraits<int, Function>>(
                    isolate);
    return local_ref_fun_map_;
}

void WrapTrader::Init(Isolate *iso, int args) {
    isolate = iso;

    // Prepare constructor template
    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
    tpl->SetClassName(String::NewFromUtf8(isolate, "WrapTrader"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    // Prototype
    tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate, "on"),
                                  FunctionTemplate::New(isolate, On)->GetFunction());
 
    tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate, "connect"),
                                  FunctionTemplate::New(isolate, Connect)->GetFunction());
    
    tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate, "reqUserLogin"),
                                  FunctionTemplate::New(isolate, ReqUserLogin)->GetFunction());

    tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate, "reqUserLogout"),
                                  FunctionTemplate::New(isolate, ReqUserLogout)->GetFunction());

    tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate, "reqSettlementInfoConfirm"),
                                  FunctionTemplate::New(isolate,
                                                        ReqSettlementInfoConfirm)->GetFunction());

    tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate, "reqQryInstrument"),
                                  FunctionTemplate::New(isolate, ReqQryInstrument)->GetFunction());

    tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate, "reqQryTradingAccount"),
                                  FunctionTemplate::New(isolate,
                                                        ReqQryTradingAccount)->GetFunction());

    tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate, "reqQryInvestorPosition"),
                                  FunctionTemplate::New(isolate,
                                                        ReqQryInvestorPosition)->GetFunction());

    tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate, "reqQryInvestorPositionDetail"),
                                  FunctionTemplate::New(isolate,
                                                        ReqQryInvestorPositionDetail)->GetFunction());

    tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate, "reqOrderInsert"),
                                  FunctionTemplate::New(isolate, ReqOrderInsert)->GetFunction());

    tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate, "reqOrderAction"),
                                  FunctionTemplate::New(isolate, ReqOrderAction)->GetFunction());

    tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate, "reqQryInstrumentMarginRate"),
                                  FunctionTemplate::New(isolate,
                                                        ReqQryInstrumentMarginRate)->GetFunction());

    tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate, "reqQryDepthMarketData"),
                                  FunctionTemplate::New(isolate,
                                                        ReqQryDepthMarketData)->GetFunction());

    tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate, "reqQrySettlementInfo"),
                                  FunctionTemplate::New(isolate,
                                                        ReqQrySettlementInfo)->GetFunction());

    tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate, "disconnect"),
                                  FunctionTemplate::New(isolate, Disposed)->GetFunction());

    tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate, "getTradingDay"),
                                  FunctionTemplate::New(isolate, GetTradingDay)->GetFunction());

    constructor.Reset(isolate, tpl->GetFunction());
    
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

void WrapTrader::New(const FunctionCallbackInfo <Value> &args) {
    if (event_map.size() == 0)
        initEventMap();

    WrapTrader *obj = new WrapTrader();
    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
}

void WrapTrader::NewInstance(const FunctionCallbackInfo <Value> &args) {
    const unsigned argc = 1;
    Local <Value> argv[argc] = {args[0]};
    Local <Function> cons = Local<Function>::New(isolate, constructor);
    Local <Context> context = isolate->GetCurrentContext();
    Local <Object> instance = cons->NewInstance(context, argc, argv).ToLocalChecked();
    args.GetReturnValue().Set(instance);
}

void WrapTrader::On(const FunctionCallbackInfo <Value> &args) {
    Isolate *isolate = args.GetIsolate();

    if (args[0]->IsUndefined() || args[1]->IsUndefined()) {
        logger_cout("Wrong FunctionCallbackInfo<Value>->event name or function");
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,
                                                                         "Wrong FunctionCallbackInfo<Value>->event name or function")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }

    WrapTrader *obj = ObjectWrap::Unwrap<WrapTrader>(args.This());

    Local <String> eventName = args[0]->ToString();
    Local <Function> cb = Local<Function>::Cast(args[1]);
    //Persistent<Function> unRecoveryCb;
    //unRecoveryCb.Reset(isolate,cb);
    String::Utf8Value eNameAscii(eventName);

    std::map<const char *, int>::iterator eIt = event_map.find(*eNameAscii);
    if (eIt == event_map.end()) {
        logger_cout("System has not register this event");
        isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(isolate, "System has no register this event")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }

    Local <Function> cIt = callback_map(isolate).Get(eIt->second);
    if (cIt != Undefined(isolate)) {
        logger_cout("Callback is defined before");
        isolate->ThrowException(
                Exception::TypeError(String::NewFromUtf8(isolate, "Callback is defined before")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }

    callback_map(isolate).Set(eIt->second, cb);
    obj->uvTrader->On(*eNameAscii, eIt->second, FunCallback);
    args.GetReturnValue().Set(Int32::New(isolate, 0));
    return;
}

void WrapTrader::Connect(const FunctionCallbackInfo <Value> &args) {
    std::string log = "wrap_trader Connect------>";
    if (args[0]->IsUndefined()) {
        logger_cout("Wrong FunctionCallbackInfo<Value>->front addr");
        isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(isolate, "Wrong FunctionCallbackInfo<Value>->front addr")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    if (!args[2]->IsNumber() || !args[3]->IsNumber()) {
        logger_cout("Wrong FunctionCallbackInfo<Value>->public or private topic type");
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,
                                                                         "Wrong FunctionCallbackInfo<Value>->public or private topic type")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    int uuid = -1;
    WrapTrader *obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
    if (!args[4]->IsUndefined() && args[4]->IsFunction()) {
        uuid = ++s_uuid;
        fun_rtncb_map(isolate).Set(uuid, Local<Function>::Cast(args[4]));
        std::string _head = std::string(log);
        logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
    }

    Local <String> frontAddr = args[0]->ToString();
    Local <String> szPath = args[1]->IsUndefined() ? String::NewFromUtf8(isolate, "t")
                                                   : args[0]->ToString();
    String::Utf8Value addrAscii(frontAddr);
    String::Utf8Value pathAscii(szPath);
    int publicTopicType = args[2]->Int32Value();
    int privateTopicType = args[3]->Int32Value();

    UVConnectField pConnectField;
    memset(&pConnectField, 0, sizeof(pConnectField));
    strcpy(pConnectField.front_addr, *addrAscii);
    strcpy(pConnectField.szPath, *pathAscii);
    pConnectField.public_topic_type = publicTopicType;
    pConnectField.private_topic_type = privateTopicType;
    obj->uvTrader->Connect(&pConnectField, FunRtnCallback, uuid);
    args.GetReturnValue().Set(Undefined(isolate));
    return;
}

void WrapTrader::ReqUserLogin(const FunctionCallbackInfo <Value> &args) {
    Isolate *isolate = args.GetIsolate();
    std::string log = "wrap_trader ReqUserLogin------>";
    if (args[0]->IsUndefined() || args[1]->IsUndefined() || args[2]->IsUndefined()) {
        std::string _head = std::string(log);
        logger_cout(_head.append(" Wrong FunctionCallbackInfo<Value>").c_str());
        isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(isolate, "Wrong FunctionCallbackInfo<Value>")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }

    int uuid = -1;
    WrapTrader *obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
    if (!args[3]->IsUndefined() && args[3]->IsFunction()) {
        uuid = ++s_uuid;
        fun_rtncb_map(isolate).Set(uuid, Local<Function>::Cast(args[3]));
        std::string _head = std::string(log);
        logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
    }

    Local <String> broker = args[0]->ToString();
    Local <String> userId = args[1]->ToString();
    Local <String> pwd = args[2]->ToString();
    String::Utf8Value brokerAscii(broker);
    String::Utf8Value userIdAscii(userId);
    String::Utf8Value pwdAscii(pwd);

    CThostFtdcReqUserLoginField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, *brokerAscii);
    strcpy(req.UserID, *userIdAscii);
    strcpy(req.Password, *pwdAscii);
    obj->uvTrader->ReqUserLogin(&req, FunRtnCallback, uuid);
    args.GetReturnValue().Set(Undefined(isolate));
    return;
}

void WrapTrader::ReqUserLogout(const FunctionCallbackInfo <Value> &args) {
    Isolate *isolate = args.GetIsolate();
    std::string log = "wrap_trader ReqUserLogout------>";

    if (args[0]->IsUndefined() || args[1]->IsUndefined()) {
        std::string _head = std::string(log);
        logger_cout(_head.append(" Wrong FunctionCallbackInfo<Value>").c_str());
        isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(isolate, "Wrong FunctionCallbackInfo<Value>")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    int uuid = -1;
    WrapTrader *obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
    if (!args[2]->IsUndefined() && args[2]->IsFunction()) {
        uuid = ++s_uuid;
        fun_rtncb_map(isolate).Set(uuid, Local<Function>::Cast(args[2]));
        std::string _head = std::string(log);
        logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
    }

    Local <String> broker = args[0]->ToString();
    Local <String> userId = args[1]->ToString();
    String::Utf8Value brokerAscii(broker);
    String::Utf8Value userIdAscii(userId);

    CThostFtdcUserLogoutField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, *brokerAscii);
    strcpy(req.UserID, *userIdAscii);
    obj->uvTrader->ReqUserLogout(&req, FunRtnCallback, uuid);
    args.GetReturnValue().Set(Undefined(isolate));
    return;
}

void WrapTrader::ReqSettlementInfoConfirm(const FunctionCallbackInfo <Value> &args) {
    Isolate *isolate = args.GetIsolate();
    std::string log = "wrap_trader ReqSettlementInfoConfirm------>";

    if (args[0]->IsUndefined() || args[1]->IsUndefined()) {
        std::string _head = std::string(log);
        logger_cout(_head.append(" Wrong FunctionCallbackInfo<Value>").c_str());
        isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(isolate, "Wrong FunctionCallbackInfo<Value>")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    int uuid = -1;
    WrapTrader *obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
    if (!args[2]->IsUndefined() && args[2]->IsFunction()) {
        uuid = ++s_uuid;
        fun_rtncb_map(isolate).Set(uuid, Local<Function>::Cast(args[2]));
        std::string _head = std::string(log);
        logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
    }

    Local <String> broker = args[0]->ToString();
    Local <String> investorId = args[1]->ToString();
    String::Utf8Value brokerAscii(broker);
    String::Utf8Value investorIdAscii(investorId);

    CThostFtdcSettlementInfoConfirmField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, *brokerAscii);
    strcpy(req.InvestorID, *investorIdAscii);
    obj->uvTrader->ReqSettlementInfoConfirm(&req, FunRtnCallback, uuid);
    args.GetReturnValue().Set(Undefined(isolate));
    return;
}

void WrapTrader::ReqQryInstrument(const FunctionCallbackInfo <Value> &args) {
    Isolate *isolate = args.GetIsolate();
    std::string log = "wrap_trader ReqQryInstrument------>";

    if (args[0]->IsUndefined()) {
        std::string _head = std::string(log);
        logger_cout(_head.append(" Wrong FunctionCallbackInfo<Value>").c_str());
        isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(isolate, "Wrong FunctionCallbackInfo<Value>")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    int uuid = -1;
    WrapTrader *obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
    if (!args[1]->IsUndefined() && args[1]->IsFunction()) {
        uuid = ++s_uuid;
        fun_rtncb_map(isolate).Set(uuid, Local<Function>::Cast(args[1]));
        std::string _head = std::string(log);
        logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
    }

    Local <String> instrumentId = args[0]->ToString();
    String::Utf8Value instrumentIdAscii(instrumentId);

    CThostFtdcQryInstrumentField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.InstrumentID, *instrumentIdAscii);
    obj->uvTrader->ReqQryInstrument(&req, FunRtnCallback, uuid);
    args.GetReturnValue().Set(Undefined(isolate));
    return;
}

void WrapTrader::ReqQryTradingAccount(const FunctionCallbackInfo <Value> &args) {
    Isolate *isolate = args.GetIsolate();
    std::string log = "wrap_trader ReqQryTradingAccount------>";

    if (args[0]->IsUndefined() || args[1]->IsUndefined()) {
        std::string _head = std::string(log);
        logger_cout(_head.append(" Wrong FunctionCallbackInfo<Value>").c_str());
        isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(isolate, "Wrong FunctionCallbackInfo<Value>")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    int uuid = -1;
    WrapTrader *obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
    if (!args[2]->IsUndefined() && args[2]->IsFunction()) {
        uuid = ++s_uuid;
        fun_rtncb_map(isolate).Set(uuid, Local<Function>::Cast(args[2]));
        std::string _head = std::string(log);
        logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
    }
    Local <String> broker = args[0]->ToString();
    Local <String> investorId = args[1]->ToString();
    String::Utf8Value brokerAscii(broker);
    String::Utf8Value investorIdAscii(investorId);

    CThostFtdcQryTradingAccountField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, *brokerAscii);
    strcpy(req.InvestorID, *investorIdAscii);
    obj->uvTrader->ReqQryTradingAccount(&req, FunRtnCallback, uuid);
    args.GetReturnValue().Set(Undefined(isolate));
    return;
}

void WrapTrader::ReqQryInvestorPosition(const FunctionCallbackInfo <Value> &args) {
    Isolate *isolate = args.GetIsolate();
    std::string log = "wrap_trader ReqQryInvestorPosition------>";

    if (args[0]->IsUndefined() || args[1]->IsUndefined() || args[2]->IsUndefined()) {
        std::string _head = std::string(log);
        logger_cout(_head.append(" Wrong FunctionCallbackInfo<Value>").c_str());
        isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(isolate, "Wrong FunctionCallbackInfo<Value>")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    int uuid = -1;
    WrapTrader *obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
    if (!args[3]->IsUndefined() && args[3]->IsFunction()) {
        uuid = ++s_uuid;
        fun_rtncb_map(isolate).Set(uuid, Local<Function>::Cast(args[3]));
        std::string _head = std::string(log);
        logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
    }
    Local <String> broker = args[0]->ToString();
    Local <String> investorId = args[1]->ToString();
    Local <String> instrumentId = args[2]->ToString();
    String::Utf8Value brokerAscii(broker);
    String::Utf8Value investorIdAscii(investorId);
    String::Utf8Value instrumentIdAscii(instrumentId);

    CThostFtdcQryInvestorPositionField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, ((std::string) * brokerAscii).c_str());
    strcpy(req.InvestorID, ((std::string) * investorIdAscii).c_str());
    strcpy(req.InstrumentID, ((std::string) * instrumentIdAscii).c_str());

    logger_cout(log.append(" ").append((std::string) * brokerAscii).append("|").append(
            (std::string) * investorIdAscii).append("|").append(
            (std::string) * instrumentIdAscii).c_str());
    obj->uvTrader->ReqQryInvestorPosition(&req, FunRtnCallback, uuid);
    args.GetReturnValue().Set(Undefined(isolate));
    return;
}

void WrapTrader::ReqQryInvestorPositionDetail(const FunctionCallbackInfo <Value> &args) {
    Isolate *isolate = args.GetIsolate();
    std::string log = "wrap_trader ReqQryInvestorPositionDetail------>";

    if (args[0]->IsUndefined() || args[1]->IsUndefined() || args[2]->IsUndefined()) {
        std::string _head = std::string(log);
        logger_cout(_head.append(" Wrong FunctionCallbackInfo<Value>").c_str());
        isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(isolate, "Wrong FunctionCallbackInfo<Value>")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    int uuid = -1;
    WrapTrader *obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
    if (!args[3]->IsUndefined() && args[3]->IsFunction()) {
        uuid = ++s_uuid;
        fun_rtncb_map(isolate).Set(uuid, Local<Function>::Cast(args[3]));
        std::string _head = std::string(log);
        logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
    }
    Local <String> broker = args[0]->ToString();
    Local <String> investorId = args[1]->ToString();
    Local <String> instrumentId = args[2]->ToString();
    String::Utf8Value brokerAscii(broker);
    String::Utf8Value investorIdAscii(investorId);
    String::Utf8Value instrumentIdAscii(instrumentId);

    CThostFtdcQryInvestorPositionDetailField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, ((std::string) * brokerAscii).c_str());
    strcpy(req.InvestorID, ((std::string) * investorIdAscii).c_str());
    strcpy(req.InstrumentID, ((std::string) * instrumentIdAscii).c_str());

    logger_cout(log.append(" ").append((std::string) * brokerAscii).append("|").append(
            (std::string) * investorIdAscii).append("|").append(
            (std::string) * instrumentIdAscii).c_str());
    obj->uvTrader->ReqQryInvestorPositionDetail(&req, FunRtnCallback, uuid);
    args.GetReturnValue().Set(Undefined(isolate));
    return;
}

void WrapTrader::ReqOrderInsert(const FunctionCallbackInfo <Value> &args) {
    Isolate *isolate = args.GetIsolate();
    std::string log = "wrap_trader ReqOrderInsert------>";

    if (args[0]->IsUndefined() || !args[0]->IsObject()) {
        std::string _head = std::string(log);
        logger_cout(_head.append(" Wrong FunctionCallbackInfo<Value>").c_str());
        isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(isolate, "Wrong FunctionCallbackInfo<Value>")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    int uuid = -1;
    WrapTrader *obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
    if (!args[1]->IsUndefined() && args[1]->IsFunction()) {
        uuid = ++s_uuid;
        fun_rtncb_map(isolate).Set(uuid, Local<Function>::Cast(args[1]));
        std::string _head = std::string(log);
        logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
    }
    Local <Object> jsonObj = args[0]->ToObject();
    Local <Value> brokerId = jsonObj->Get(v8::String::NewFromUtf8(isolate, "brokerId"));
    if (brokerId->IsUndefined()) {
        isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(isolate, "Wrong FunctionCallbackInfo<Value>->brokerId")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    String::Utf8Value brokerId_(brokerId->ToString());
    Local <Value> investorId = jsonObj->Get(v8::String::NewFromUtf8(isolate, "investorId"));
    if (investorId->IsUndefined()) {
        isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(isolate, "Wrong FunctionCallbackInfo<Value>->investorId")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    String::Utf8Value investorId_(investorId->ToString());
    Local <Value> instrumentId = jsonObj->Get(v8::String::NewFromUtf8(isolate, "instrumentId"));
    if (instrumentId->IsUndefined()) {
        isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(isolate, "Wrong FunctionCallbackInfo<Value>->instrumentId")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    String::Utf8Value instrumentId_(instrumentId->ToString());
    Local <Value> priceType = jsonObj->Get(v8::String::NewFromUtf8(isolate, "priceType"));
    if (priceType->IsUndefined()) {
        isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(isolate, "Wrong FunctionCallbackInfo<Value>->priceType")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    String::Utf8Value priceType_(priceType->ToString());
    Local <Value> direction = jsonObj->Get(v8::String::NewFromUtf8(isolate, "direction"));
    if (direction->IsUndefined()) {
        isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(isolate, "Wrong FunctionCallbackInfo<Value>->direction")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    String::Utf8Value direction_(direction->ToString());
    Local <Value> combOffsetFlag = jsonObj->Get(v8::String::NewFromUtf8(isolate, "combOffsetFlag"));
    if (combOffsetFlag->IsUndefined()) {
        isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(isolate, "Wrong FunctionCallbackInfo<Value>->combOffsetFlag")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    String::Utf8Value combOffsetFlag_(combOffsetFlag->ToString());
    Local <Value> combHedgeFlag = jsonObj->Get(v8::String::NewFromUtf8(isolate, "combHedgeFlag"));
    if (combHedgeFlag->IsUndefined()) {
        isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(isolate, "Wrong FunctionCallbackInfo<Value>->combHedgeFlag")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    String::Utf8Value combHedgeFlag_(combHedgeFlag->ToString());
    Local <Value> vlimitPrice = jsonObj->Get(v8::String::NewFromUtf8(isolate, "limitPrice"));
    if (vlimitPrice->IsUndefined()) {
        isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(isolate, "Wrong FunctionCallbackInfo<Value>->limitPrice")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    double limitPrice = vlimitPrice->NumberValue();
    Local <Value> vvolumeTotalOriginal = jsonObj->Get(
            v8::String::NewFromUtf8(isolate, "volumeTotalOriginal"));
    if (vvolumeTotalOriginal->IsUndefined()) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,
                                                                         "Wrong FunctionCallbackInfo<Value>->volumeTotalOriginal")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    int32_t volumeTotalOriginal = vvolumeTotalOriginal->Int32Value();
    Local <Value> timeCondition = jsonObj->Get(v8::String::NewFromUtf8(isolate, "timeCondition"));
    if (timeCondition->IsUndefined()) {
        isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(isolate, "Wrong FunctionCallbackInfo<Value>->timeCondition")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    String::Utf8Value timeCondition_(timeCondition->ToString());
    Local <Value> volumeCondition = jsonObj->Get(
            v8::String::NewFromUtf8(isolate, "volumeCondition"));
    if (volumeCondition->IsUndefined()) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,
                                                                         "Wrong FunctionCallbackInfo<Value>->volumeCondition")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    String::Utf8Value volumeCondition_(volumeCondition->ToString());
    Local <Value> vminVolume = jsonObj->Get(v8::String::NewFromUtf8(isolate, "minVolume"));
    if (vminVolume->IsUndefined()) {
        isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(isolate, "Wrong FunctionCallbackInfo<Value>->minVolume")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    int32_t minVolume = vminVolume->Int32Value();
    Local <Value> forceCloseReason = jsonObj->Get(
            v8::String::NewFromUtf8(isolate, "forceCloseReason"));
    if (forceCloseReason->IsUndefined()) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,
                                                                         "Wrong FunctionCallbackInfo<Value>->forceCloseReason")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    String::Utf8Value forceCloseReason_(forceCloseReason->ToString());
    Local <Value> visAutoSuspend = jsonObj->Get(v8::String::NewFromUtf8(isolate, "isAutoSuspend"));
    if (visAutoSuspend->IsUndefined()) {
        isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(isolate, "Wrong FunctionCallbackInfo<Value>->isAutoSuspend")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    int32_t isAutoSuspend = visAutoSuspend->Int32Value();
    Local <Value> vuserForceClose = jsonObj->Get(
            v8::String::NewFromUtf8(isolate, "userForceClose"));
    if (vuserForceClose->IsUndefined()) {
        isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(isolate, "Wrong FunctionCallbackInfo<Value>->userForceClose")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    int32_t userForceClose = vuserForceClose->Int32Value();

    CThostFtdcInputOrderField req;
    memset(&req, 0, sizeof(req));
    log.append(" ");

    Local <Value> orderRef = jsonObj->Get(v8::String::NewFromUtf8(isolate, "orderRef"));
    if (!orderRef->IsUndefined()) {
        String::Utf8Value orderRef_(orderRef->ToString());
        strcpy(req.OrderRef, ((std::string) * orderRef_).c_str());
        log.append("orderRef:").append((std::string) * orderRef_).append("|");
    }

    Local <Value> vstopPrice = jsonObj->Get(v8::String::NewFromUtf8(isolate, "stopPrice"));
    if (!vstopPrice->IsUndefined()) {
        double stopPrice = vstopPrice->NumberValue();
        req.StopPrice = stopPrice;
        log.append("stopPrice:").append(to_string(stopPrice)).append("|");
    }
    Local <Value> contingentCondition = jsonObj->Get(
            v8::String::NewFromUtf8(isolate, "contingentCondition"));
    if (!contingentCondition->IsUndefined()) {
        String::Utf8Value contingentCondition_(contingentCondition->ToString());
        req.ContingentCondition = ((std::string) * contingentCondition_)[0];
        log.append("contingentCondition:").append((std::string) * contingentCondition_).append("|");
    }

    strcpy(req.BrokerID, ((std::string) * brokerId_).c_str());
    strcpy(req.InvestorID, ((std::string) * investorId_).c_str());
    strcpy(req.InstrumentID, ((std::string) * instrumentId_).c_str());
    req.OrderPriceType = ((std::string) * priceType_)[0];
    req.Direction = ((std::string) * direction_)[0];
    req.CombOffsetFlag[0] = ((std::string) * combOffsetFlag_)[0];
    req.CombHedgeFlag[0] = ((std::string) * combHedgeFlag_)[0];
    req.LimitPrice = limitPrice;
    req.VolumeTotalOriginal = volumeTotalOriginal;
    req.TimeCondition = ((std::string) * timeCondition_)[0];
    req.VolumeCondition = ((std::string) * volumeCondition_)[0];
    req.MinVolume = minVolume;
    req.ForceCloseReason = ((std::string) * forceCloseReason_)[0];
    req.IsAutoSuspend = isAutoSuspend;
    req.UserForceClose = userForceClose;
    logger_cout(log.
            append("brokerID:").append((std::string) * brokerId_).append("|").
            append("investorID:").append((std::string) * investorId_).append("|").
            append("instrumentID:").append((std::string) * instrumentId_).append("|").
            append("priceType:").append((std::string) * priceType_).append("|").
            append("direction:").append((std::string) * direction_).append("|").
            append("comboffsetFlag:").append((std::string) * combOffsetFlag_).append("|").
            append("combHedgeFlag:").append((std::string) * combHedgeFlag_).append("|").
            append("limitPrice:").append(to_string(limitPrice)).append("|").
            append("volumnTotalOriginal:").append(to_string(volumeTotalOriginal)).append("|").
            append("timeCondition:").append((std::string) * timeCondition_).append("|").
            append("volumeCondition:").append((std::string) * volumeCondition_).append("|").
            append("minVolume:").append(to_string(minVolume)).append("|").
            append("forceCloseReason:").append((std::string) * forceCloseReason_).append("|").
            append("isAutoSuspend:").append(to_string(isAutoSuspend)).append("|").
            append("useForceClose:").append(to_string(userForceClose)).append("|").c_str());
    obj->uvTrader->ReqOrderInsert(&req, FunRtnCallback, uuid);
    args.GetReturnValue().Set(Undefined(isolate));
    return;
}

void WrapTrader::ReqOrderAction(const FunctionCallbackInfo <Value> &args) {
    Isolate *isolate = args.GetIsolate();
    std::string log = "wrap_trader ReqOrderAction------>";

    if (args[0]->IsUndefined() || !args[0]->IsObject()) {
        std::string _head = std::string(log);
        logger_cout(_head.append(" Wrong FunctionCallbackInfo<Value>").c_str());
        isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(isolate, "Wrong FunctionCallbackInfo<Value>")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    int uuid = -1;
    WrapTrader *obj = ObjectWrap::Unwrap<WrapTrader>(args.This());

    if (!args[1]->IsUndefined() && args[1]->IsFunction()) {
        uuid = ++s_uuid;
        fun_rtncb_map(isolate).Set(uuid, Local<Function>::Cast(args[1]));
        std::string _head = std::string(log);
        logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
    }

    Local <Object> jsonObj = args[0]->ToObject();
    Local <Value> vbrokerId = jsonObj->Get(v8::String::NewFromUtf8(isolate, "brokerId"));
    if (vbrokerId->IsUndefined()) {
        isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(isolate, "Wrong FunctionCallbackInfo<Value>->brokerId")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    String::Utf8Value brokerId_(vbrokerId->ToString());
    Local <Value> vinvestorId = jsonObj->Get(v8::String::NewFromUtf8(isolate, "investorId"));
    if (vinvestorId->IsUndefined()) {
        isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(isolate, "Wrong FunctionCallbackInfo<Value>->investorId")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    String::Utf8Value investorId_(vinvestorId->ToString());
    Local <Value> vinstrumentId = jsonObj->Get(v8::String::NewFromUtf8(isolate, "instrumentId"));
    if (vinstrumentId->IsUndefined()) {
        isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(isolate, "Wrong FunctionCallbackInfo<Value>->instrumentId")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    String::Utf8Value instrumentId_(vinstrumentId->ToString());
    Local <Value> vactionFlag = jsonObj->Get(v8::String::NewFromUtf8(isolate, "actionFlag"));
    if (vactionFlag->IsUndefined()) {
        isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(isolate, "Wrong FunctionCallbackInfo<Value>->actionFlag")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    int32_t actionFlag = vactionFlag->Int32Value();

    CThostFtdcInputOrderActionField req;
    memset(&req, 0, sizeof(req));

    log.append(" ");
    Local <Value> vorderRef = jsonObj->Get(v8::String::NewFromUtf8(isolate, "orderRef"));
    if (!vorderRef->IsUndefined()) {
        String::Utf8Value orderRef_(vorderRef->ToString());
        strcpy(req.OrderRef, ((std::string) * orderRef_).c_str());
        log.append((std::string) * orderRef_).append("|");
    }
    Local <Value> vfrontId = jsonObj->Get(v8::String::NewFromUtf8(isolate, "frontId"));
    if (!vfrontId->IsUndefined()) {
        int32_t frontId = vfrontId->Int32Value();
        req.FrontID = frontId;
        log.append(to_string(frontId)).append("|");
    }
    Local <Value> vsessionId = jsonObj->Get(v8::String::NewFromUtf8(isolate, "sessionId"));
    if (!vsessionId->IsUndefined()) {
        int32_t sessionId = vsessionId->Int32Value();
        req.SessionID = sessionId;
        log.append(to_string(sessionId)).append("|");
    }
    Local <Value> vexchangeID = jsonObj->Get(v8::String::NewFromUtf8(isolate, "exchangeID"));
    if (!vexchangeID->IsUndefined()) {
        String::Utf8Value exchangeID_(vexchangeID->ToString());
        strcpy(req.ExchangeID, ((std::string) * exchangeID_).c_str());
        log.append((std::string) * exchangeID_).append("|");
    }
    Local <Value> vorderSysID = jsonObj->Get(v8::String::NewFromUtf8(isolate, "orderSysID"));
    if (vorderSysID->IsUndefined()) {
        String::Utf8Value orderSysID_(vorderSysID->ToString());
        strcpy(req.OrderSysID, ((std::string) * orderSysID_).c_str());
        log.append((std::string) * orderSysID_).append("|");
    }

    strcpy(req.BrokerID, ((std::string) * brokerId_).c_str());
    strcpy(req.InvestorID, ((std::string) * investorId_).c_str());
    req.ActionFlag = actionFlag;
    strcpy(req.InstrumentID, ((std::string) * instrumentId_).c_str());
    logger_cout(log.
            append((std::string) * brokerId_).append("|").
            append((std::string) * investorId_).append("|").
            append((std::string) * instrumentId_).append("|").
            append(to_string(actionFlag)).append("|").c_str());

    obj->uvTrader->ReqOrderAction(&req, FunRtnCallback, uuid);
    args.GetReturnValue().Set(Undefined(isolate));
    return;
}

void WrapTrader::ReqQryInstrumentMarginRate(const FunctionCallbackInfo <Value> &args) {
    Isolate *isolate = args.GetIsolate();
    std::string log = "wrap_trader ReqQryInstrumentMarginRate------>";

    if (args[0]->IsUndefined() || args[1]->IsUndefined() || args[2]->IsUndefined() ||
        args[3]->IsUndefined()) {
        std::string _head = std::string(log);
        logger_cout(_head.append(" Wrong FunctionCallbackInfo<Value>").c_str());
        isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(isolate, "Wrong FunctionCallbackInfo<Value>")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    int uuid = -1;
    WrapTrader *obj = ObjectWrap::Unwrap<WrapTrader>(args.This());

    if (!args[4]->IsUndefined() && args[4]->IsFunction()) {
        uuid = ++s_uuid;
        fun_rtncb_map(isolate).Set(uuid, Local<Function>::Cast(args[4]));
        std::string _head = std::string(log);
        logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
    }

    Local <String> broker = args[0]->ToString();
    Local <String> investorId = args[1]->ToString();
    Local <String> instrumentId = args[2]->ToString();
    int32_t hedgeFlag = args[3]->Int32Value();
    String::Utf8Value brokerAscii(broker);
    String::Utf8Value investorIdAscii(investorId);
    String::Utf8Value instrumentIdAscii(instrumentId);

    CThostFtdcQryInstrumentMarginRateField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, ((std::string) * brokerAscii).c_str());
    strcpy(req.InvestorID, ((std::string) * investorIdAscii).c_str());
    strcpy(req.InstrumentID, ((std::string) * instrumentIdAscii).c_str());
    req.HedgeFlag = hedgeFlag;
    logger_cout(log.append(" ").
            append((std::string) * brokerAscii).append("|").
            append((std::string) * investorIdAscii).append("|").
            append((std::string) * instrumentIdAscii).append("|").
            append(to_string(hedgeFlag)).append("|").c_str());

    obj->uvTrader->ReqQryInstrumentMarginRate(&req, FunRtnCallback, uuid);
    args.GetReturnValue().Set(Undefined(isolate));
    return;
}

void WrapTrader::ReqQryDepthMarketData(const FunctionCallbackInfo <Value> &args) {
    Isolate *isolate = args.GetIsolate();
    std::string log = "wrap_trader ReqQryDepthMarketData------>";

    if (args[0]->IsUndefined()) {
        std::string _head = std::string(log);
        logger_cout(_head.append(" Wrong FunctionCallbackInfo<Value>").c_str());
        isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(isolate, "Wrong FunctionCallbackInfo<Value>")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    int uuid = -1;
    WrapTrader *obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
    if (!args[1]->IsUndefined() && args[1]->IsFunction()) {
        uuid = ++s_uuid;
        fun_rtncb_map(isolate).Set(uuid, Local<Function>::Cast(args[1]));
        std::string _head = std::string(log);
        logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
    }

    Local <String> instrumentId = args[0]->ToString();
    String::Utf8Value instrumentIdAscii(instrumentId);

    CThostFtdcQryDepthMarketDataField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.InstrumentID, ((std::string) * instrumentIdAscii).c_str());
    logger_cout(log.append(" ").
            append((std::string) * instrumentIdAscii).append("|").c_str());
    obj->uvTrader->ReqQryDepthMarketData(&req, FunRtnCallback, uuid);
    args.GetReturnValue().Set(Undefined(isolate));
    return;
}

void WrapTrader::ReqQrySettlementInfo(const FunctionCallbackInfo <Value> &args) {
    Isolate *isolate = args.GetIsolate();
    std::string log = "wrap_trader ReqQrySettlementInfo------>";

    if (args[0]->IsUndefined() || args[1]->IsUndefined() || args[2]->IsUndefined()) {
        std::string _head = std::string(log);
        logger_cout(_head.append(" Wrong FunctionCallbackInfo<Value>").c_str());
        isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(isolate, "Wrong FunctionCallbackInfo<Value>")));
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    int uuid = -1;
    WrapTrader *obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
    if (!args[3]->IsUndefined() && args[3]->IsFunction()) {
        uuid = ++s_uuid;
        fun_rtncb_map(isolate).Set(uuid, Local<Function>::Cast(args[3]));
        std::string _head = std::string(log);
        logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
    }

    Local <String> broker = args[0]->ToString();
    Local <String> investorId = args[1]->ToString();
    Local <String> tradingDay = args[2]->ToString();
    String::Utf8Value brokerAscii(broker);
    String::Utf8Value investorIdAscii(investorId);
    String::Utf8Value tradingDayAscii(tradingDay);

    CThostFtdcQrySettlementInfoField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, ((std::string) * brokerAscii).c_str());
    strcpy(req.InvestorID, ((std::string) * investorIdAscii).c_str());
    strcpy(req.TradingDay, ((std::string) * tradingDayAscii).c_str());
    logger_cout(log.append(" ").
            append((std::string) * brokerAscii).append("|").
            append((std::string) * investorIdAscii).append("|").
            append((std::string) * tradingDayAscii).append("|").c_str());

    obj->uvTrader->ReqQrySettlementInfo(&req, FunRtnCallback, uuid);
    args.GetReturnValue().Set(Undefined(isolate));
    return;
}

void WrapTrader::Disposed(const FunctionCallbackInfo <Value> &args) {
    Isolate *isolate = args.GetIsolate();
    WrapTrader *obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
    obj->uvTrader->Disconnect();
    //zhangls clear is enouh  
    //Local<Function> callback_it = callback_map(isolate).begin();
    //while (callback_it != Undefined(isolate)) {
    //callback_it->second.Dispose();
    //callback_it++;
    //}
    event_map.clear();
    callback_map(isolate).Clear();
    fun_rtncb_map(isolate).Clear();
    delete obj->uvTrader;
    obj->uvTrader = NULL;
    logger_cout("wrap_trader Disposed------>wrap disposed");
    args.GetReturnValue().Set(Undefined(isolate));
    return;
}

void WrapTrader::GetTradingDay(const FunctionCallbackInfo <Value> &args) {
    Isolate *isolate = args.GetIsolate();
    WrapTrader *obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
    const char *tradingDay = obj->uvTrader->GetTradingDay();
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, tradingDay));
    return;
}

void WrapTrader::FunCallback(CbRtnField *data) {
    Local <Function> cIt = callback_map(isolate).Get(data->eFlag);
    if (cIt == Undefined(isolate))
        return;

    switch (data->eFlag) {
        case T_ON_CONNECT: {
            Local <Value> argv[1] = {Local<Value>::New(isolate, Undefined(isolate))};
            cIt->Call(isolate->GetCurrentContext()->Global(), 1, argv);
            break;
        }
        case T_ON_DISCONNECTED: {
            Local <Value> argv[1] = {Int32::New(isolate, data->nReason)};
            cIt->Call(isolate->GetCurrentContext()->Global(), 1, argv);
            break;
        }
        case T_ON_RSPUSERLOGIN: {
            Local <Value> argv[4];
            pkg_cb_userlogin(data, argv);
            cIt->Call(isolate->GetCurrentContext()->Global(), 4, argv);
            break;
        }
        case T_ON_RSPUSERLOGOUT: {
            Local <Value> argv[4];
            pkg_cb_userlogout(data, argv);
            cIt->Call(isolate->GetCurrentContext()->Global(), 4, argv);
            break;
        }
        case T_ON_RSPINFOCONFIRM: {
            Local <Value> argv[4];
            pkg_cb_confirm(data, argv);
            cIt->Call(isolate->GetCurrentContext()->Global(), 4, argv);
            break;
        }
        case T_ON_RSPINSERT: {
            Local <Value> argv[4];
            pkg_cb_orderinsert(data, argv);
            cIt->Call(isolate->GetCurrentContext()->Global(), 4, argv);
            break;
        }
        case T_ON_ERRINSERT: {
            Local <Value> argv[2];
            pkg_cb_errorderinsert(data, argv);
            cIt->Call(isolate->GetCurrentContext()->Global(), 2, argv);
            break;
        }
        case T_ON_RSPACTION: {
            Local <Value> argv[4];
            pkg_cb_orderaction(data, argv);
            cIt->Call(isolate->GetCurrentContext()->Global(), 4, argv);
            break;
        }
        case T_ON_ERRACTION: {
            Local <Value> argv[2];
            pkg_cb_errorderaction(data, argv);
            cIt->Call(isolate->GetCurrentContext()->Global(), 2, argv);

            break;
        }
        case T_ON_RQORDER: {
            Local <Value> argv[4];
            pkg_cb_rspqryorder(data, argv);
            cIt->Call(isolate->GetCurrentContext()->Global(), 4, argv);
            break;
        }
        case T_ON_RTNORDER: {
            Local <Value> argv[1];
            pkg_cb_rtnorder(data, argv);
            cIt->Call(isolate->GetCurrentContext()->Global(), 1, argv);

            break;
        }
        case T_ON_RQTRADE: {
            Local <Value> argv[4];
            pkg_cb_rqtrade(data, argv);
            cIt->Call(isolate->GetCurrentContext()->Global(), 4, argv);

            break;
        }
        case T_ON_RTNTRADE: {
            Local <Value> argv[1];
            pkg_cb_rtntrade(data, argv);
            cIt->Call(isolate->GetCurrentContext()->Global(), 1, argv);

            break;
        }
        case T_ON_RQINVESTORPOSITION: {
            Local <Value> argv[4];
            pkg_cb_rqinvestorposition(data, argv);
            cIt->Call(isolate->GetCurrentContext()->Global(), 4, argv);

            break;
        }
        case T_ON_RQINVESTORPOSITIONDETAIL: {
            Local <Value> argv[4];
            pkg_cb_rqinvestorpositiondetail(data, argv);
            cIt->Call(isolate->GetCurrentContext()->Global(), 4, argv);

            break;
        }
        case T_ON_RQTRADINGACCOUNT: {
            Local <Value> argv[4];
            pkg_cb_rqtradingaccount(data, argv);
            cIt->Call(isolate->GetCurrentContext()->Global(), 4, argv);

            break;
        }
        case T_ON_RQINSTRUMENT: {
            Local <Value> argv[4];
            pkg_cb_rqinstrument(data, argv);
            cIt->Call(isolate->GetCurrentContext()->Global(), 4, argv);

            break;
        }
        case T_ON_RQDEPTHMARKETDATA: {
            Local <Value> argv[4];
            pkg_cb_rqdepthmarketdata(data, argv);
            cIt->Call(isolate->GetCurrentContext()->Global(), 4, argv);

            break;
        }
        case T_ON_RQSETTLEMENTINFO: {
            Local <Value> argv[4];
            pkg_cb_rqsettlementinfo(data, argv);
            cIt->Call(isolate->GetCurrentContext()->Global(), 4, argv);
            break;
        }
        case T_ON_RSPERROR: {
            Local <Value> argv[3];
            pkg_cb_rsperror(data, argv);
            cIt->Call(isolate->GetCurrentContext()->Global(), 3, argv);

            break;
        }
    }
}

void WrapTrader::FunRtnCallback(int result, void *baton) {
    LookupCtpApiBaton *tmp = static_cast<LookupCtpApiBaton *>(baton);
    if (tmp->uuid != -1) {
        Local <Function> it = fun_rtncb_map(isolate).Get(tmp->uuid);
        Local <Value> argv[2] = {Local<Value>::New(isolate, Int32::New(isolate, tmp->nResult)),
                                 Local<Value>::New(isolate, Int32::New(isolate, tmp->iRequestID))};
        it->Call(isolate->GetCurrentContext()->Global(), 2, argv);
        fun_rtncb_map(isolate).Remove(tmp->uuid);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void WrapTrader::pkg_cb_userlogin(CbRtnField *data, Local <Value> *cbArray) {
    *cbArray = Int32::New(isolate, data->nRequestID);
    *(cbArray + 1) = Boolean::New(isolate, data->bIsLast)->ToBoolean();
    if (data->rtnField) {
        CThostFtdcRspUserLoginField *pRspUserLogin = static_cast<CThostFtdcRspUserLoginField *>(data->rtnField);
        Local <Object> jsonRtn = Object::New(isolate);
        jsonRtn->Set(String::NewFromUtf8(isolate, "TradingDay"),
                     String::NewFromUtf8(isolate, pRspUserLogin->TradingDay));
        jsonRtn->Set(String::NewFromUtf8(isolate, "LoginTime"),
                     String::NewFromUtf8(isolate, pRspUserLogin->LoginTime));
        jsonRtn->Set(String::NewFromUtf8(isolate, "BrokerID"),
                     String::NewFromUtf8(isolate, pRspUserLogin->BrokerID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "UserID"),
                     String::NewFromUtf8(isolate, pRspUserLogin->UserID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "SystemName"),
                     String::NewFromUtf8(isolate, pRspUserLogin->SystemName));
        jsonRtn->Set(String::NewFromUtf8(isolate, "FrontID"),
                     Int32::New(isolate, pRspUserLogin->FrontID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "SessionID"),
                     Int32::New(isolate, pRspUserLogin->SessionID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "MaxOrderRef"),
                     String::NewFromUtf8(isolate, pRspUserLogin->MaxOrderRef));
        jsonRtn->Set(String::NewFromUtf8(isolate, "SHFETime"),
                     String::NewFromUtf8(isolate, pRspUserLogin->SHFETime));
        jsonRtn->Set(String::NewFromUtf8(isolate, "DCETime"),
                     String::NewFromUtf8(isolate, pRspUserLogin->DCETime));
        jsonRtn->Set(String::NewFromUtf8(isolate, "CZCETime"),
                     String::NewFromUtf8(isolate, pRspUserLogin->CZCETime));
        jsonRtn->Set(String::NewFromUtf8(isolate, "FFEXTime"),
                     String::NewFromUtf8(isolate, pRspUserLogin->FFEXTime));
        jsonRtn->Set(String::NewFromUtf8(isolate, "INETime"),
                     String::NewFromUtf8(isolate, pRspUserLogin->INETime));
        *(cbArray + 2) = jsonRtn;
    }
    else {
        *(cbArray + 2) = Local<Value>::New(isolate, Undefined(isolate));
    }

    *(cbArray + 3) = pkg_rspinfo(data->rspInfo);
    return;
}

void WrapTrader::pkg_cb_userlogout(CbRtnField *data, Local <Value> *cbArray) {
    *cbArray = Int32::New(isolate, data->nRequestID);
    *(cbArray + 1) = Boolean::New(isolate, data->bIsLast)->ToBoolean();
    if (data->rtnField) {
        CThostFtdcRspUserLoginField *pRspUserLogin = static_cast<CThostFtdcRspUserLoginField *>(data->rtnField);

        Local <Object> jsonRtn = Object::New(isolate);
        jsonRtn->Set(String::NewFromUtf8(isolate, "BrokerID"),
                     String::NewFromUtf8(isolate, pRspUserLogin->BrokerID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "UserID"),
                     String::NewFromUtf8(isolate, pRspUserLogin->UserID));
        *(cbArray + 2) = jsonRtn;
    }
    else {
        *(cbArray + 2) = Local<Value>::New(isolate, Undefined(isolate));
    }
    *(cbArray + 3) = pkg_rspinfo(data->rspInfo);
    return;
}

void WrapTrader::pkg_cb_confirm(CbRtnField *data, Local <Value> *cbArray) {
    *cbArray = Int32::New(isolate, data->nRequestID);
    *(cbArray + 1) = Boolean::New(isolate, data->bIsLast)->ToBoolean();
    if (data->rtnField) {
        CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm = static_cast<CThostFtdcSettlementInfoConfirmField *>(data->rtnField);
        Local <Object> jsonRtn = Object::New(isolate);
        jsonRtn->Set(String::NewFromUtf8(isolate, "BrokerID"),
                     String::NewFromUtf8(isolate, pSettlementInfoConfirm->BrokerID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "InvestorID"),
                     String::NewFromUtf8(isolate, pSettlementInfoConfirm->InvestorID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ConfirmDate"),
                     String::NewFromUtf8(isolate, pSettlementInfoConfirm->ConfirmDate));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ConfirmTime"),
                     String::NewFromUtf8(isolate, pSettlementInfoConfirm->ConfirmTime));
        *(cbArray + 2) = jsonRtn;
    }
    else {
        *(cbArray + 2) = Local<Value>::New(isolate, Undefined(isolate));
    }
    *(cbArray + 3) = pkg_rspinfo(data->rspInfo);
    return;
}

void WrapTrader::pkg_cb_orderinsert(CbRtnField *data, Local <Value> *cbArray) {
    *cbArray = Int32::New(isolate, data->nRequestID);
    *(cbArray + 1) = Boolean::New(isolate, data->bIsLast)->ToBoolean();
    if (data->rtnField) {
        CThostFtdcInputOrderField *pInputOrder = static_cast<CThostFtdcInputOrderField *>(data->rtnField);
        Local <Object> jsonRtn = Object::New(isolate);
        jsonRtn->Set(String::NewFromUtf8(isolate, "BrokerID"),
                     String::NewFromUtf8(isolate, pInputOrder->BrokerID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "InvestorID"),
                     String::NewFromUtf8(isolate, pInputOrder->InvestorID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "InstrumentID"),
                     String::NewFromUtf8(isolate, pInputOrder->InstrumentID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OrderRef"),
                     String::NewFromUtf8(isolate, pInputOrder->OrderRef));
        jsonRtn->Set(String::NewFromUtf8(isolate, "UserID"),
                     String::NewFromUtf8(isolate, pInputOrder->UserID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OrderPriceType"), String::NewFromUtf8(isolate,
                                                                                         charto_string(
                                                                                                 pInputOrder->OrderPriceType).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "Direction"), String::NewFromUtf8(isolate,
                                                                                    charto_string(
                                                                                            pInputOrder->Direction).c_str()));  //var charval = String.fromCharCode(asciival);
        jsonRtn->Set(String::NewFromUtf8(isolate, "CombOffsetFlag"),
                     String::NewFromUtf8(isolate, pInputOrder->CombOffsetFlag));
        jsonRtn->Set(String::NewFromUtf8(isolate, "CombHedgeFlag"),
                     String::NewFromUtf8(isolate, pInputOrder->CombHedgeFlag));
        jsonRtn->Set(String::NewFromUtf8(isolate, "LimitPrice"),
                     Number::New(isolate, pInputOrder->LimitPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate, "VolumeTotalOriginal"),
                     Int32::New(isolate, pInputOrder->VolumeTotalOriginal));
        jsonRtn->Set(String::NewFromUtf8(isolate, "TimeCondition"), String::NewFromUtf8(isolate,
                                                                                        charto_string(
                                                                                                pInputOrder->TimeCondition).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "GTDDate"),
                     String::NewFromUtf8(isolate, pInputOrder->GTDDate));
        jsonRtn->Set(String::NewFromUtf8(isolate, "VolumeCondition"), String::NewFromUtf8(isolate,
                                                                                          charto_string(
                                                                                                  pInputOrder->VolumeCondition).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "MinVolume"),
                     Int32::New(isolate, pInputOrder->MinVolume));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ContingentCondition"),
                     String::NewFromUtf8(isolate,
                                         charto_string(pInputOrder->ContingentCondition).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "StopPrice"),
                     Number::New(isolate, pInputOrder->StopPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ForceCloseReason"), String::NewFromUtf8(isolate,
                                                                                           charto_string(
                                                                                                   pInputOrder->ForceCloseReason).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "IsAutoSuspend"),
                     Int32::New(isolate, pInputOrder->IsAutoSuspend));
        jsonRtn->Set(String::NewFromUtf8(isolate, "BusinessUnit"),
                     String::NewFromUtf8(isolate, pInputOrder->BusinessUnit));
        jsonRtn->Set(String::NewFromUtf8(isolate, "RequestID"),
                     Int32::New(isolate, pInputOrder->RequestID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "UserForceClose"),
                     Int32::New(isolate, pInputOrder->UserForceClose));
        jsonRtn->Set(String::NewFromUtf8(isolate, "IsSwapOrder"),
                     Int32::New(isolate, pInputOrder->IsSwapOrder));
        *(cbArray + 2) = jsonRtn;
    }
    else {
        *(cbArray + 2) = Local<Value>::New(isolate, Undefined(isolate));
    }
    *(cbArray + 3) = pkg_rspinfo(data->rspInfo);
    return;
}

void WrapTrader::pkg_cb_errorderinsert(CbRtnField *data, Local <Value> *cbArray) {
    if (data->rtnField) {
        CThostFtdcInputOrderField *pInputOrder = static_cast<CThostFtdcInputOrderField *>(data->rtnField);
        Local <Object> jsonRtn = Object::New(isolate);
        jsonRtn->Set(String::NewFromUtf8(isolate, "BrokerID"),
                     String::NewFromUtf8(isolate, pInputOrder->BrokerID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "InvestorID"),
                     String::NewFromUtf8(isolate, pInputOrder->InvestorID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "InstrumentID"),
                     String::NewFromUtf8(isolate, pInputOrder->InstrumentID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OrderRef"),
                     String::NewFromUtf8(isolate, pInputOrder->OrderRef));
        jsonRtn->Set(String::NewFromUtf8(isolate, "UserID"),
                     String::NewFromUtf8(isolate, pInputOrder->UserID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OrderPriceType"), String::NewFromUtf8(isolate,
                                                                                         charto_string(
                                                                                                 pInputOrder->OrderPriceType).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "Direction"), String::NewFromUtf8(isolate,
                                                                                    charto_string(
                                                                                            pInputOrder->Direction).c_str()));  //var charval = String.fromCharCode(asciival);
        jsonRtn->Set(String::NewFromUtf8(isolate, "CombOffsetFlag"),
                     String::NewFromUtf8(isolate, pInputOrder->CombOffsetFlag));
        jsonRtn->Set(String::NewFromUtf8(isolate, "CombHedgeFlag"),
                     String::NewFromUtf8(isolate, pInputOrder->CombHedgeFlag));
        jsonRtn->Set(String::NewFromUtf8(isolate, "LimitPrice"),
                     Number::New(isolate, pInputOrder->LimitPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate, "VolumeTotalOriginal"),
                     Int32::New(isolate, pInputOrder->VolumeTotalOriginal));
        jsonRtn->Set(String::NewFromUtf8(isolate, "TimeCondition"), String::NewFromUtf8(isolate,
                                                                                        charto_string(
                                                                                                pInputOrder->TimeCondition).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "GTDDate"),
                     String::NewFromUtf8(isolate, pInputOrder->GTDDate));
        jsonRtn->Set(String::NewFromUtf8(isolate, "VolumeCondition"), String::NewFromUtf8(isolate,
                                                                                          charto_string(
                                                                                                  pInputOrder->VolumeCondition).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "MinVolume"),
                     Int32::New(isolate, pInputOrder->MinVolume));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ContingentCondition"),
                     String::NewFromUtf8(isolate,
                                         charto_string(pInputOrder->ContingentCondition).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "StopPrice"),
                     Number::New(isolate, pInputOrder->StopPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ForceCloseReason"), String::NewFromUtf8(isolate,
                                                                                           charto_string(
                                                                                                   pInputOrder->ForceCloseReason).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "IsAutoSuspend"),
                     Int32::New(isolate, pInputOrder->IsAutoSuspend));
        jsonRtn->Set(String::NewFromUtf8(isolate, "BusinessUnit"),
                     String::NewFromUtf8(isolate, pInputOrder->BusinessUnit));
        jsonRtn->Set(String::NewFromUtf8(isolate, "RequestID"),
                     Int32::New(isolate, pInputOrder->RequestID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "UserForceClose"),
                     Int32::New(isolate, pInputOrder->UserForceClose));
        jsonRtn->Set(String::NewFromUtf8(isolate, "IsSwapOrder"),
                     Int32::New(isolate, pInputOrder->IsSwapOrder));
        *cbArray = jsonRtn;
    }
    else {
        *cbArray = Local<Value>::New(isolate, Undefined(isolate));
    }
    *(cbArray + 1) = pkg_rspinfo(data->rspInfo);
    return;
}

void WrapTrader::pkg_cb_orderaction(CbRtnField *data, Local <Value> *cbArray) {
    *cbArray = Int32::New(isolate, data->nRequestID);
    *(cbArray + 1) = Boolean::New(isolate, data->bIsLast)->ToBoolean();
    if (data->rtnField) {
        CThostFtdcInputOrderActionField *pInputOrderAction = static_cast<CThostFtdcInputOrderActionField *>(data->rtnField);
        Local <Object> jsonRtn = Object::New(isolate);
        jsonRtn->Set(String::NewFromUtf8(isolate, "BrokerID"),
                     String::NewFromUtf8(isolate, pInputOrderAction->BrokerID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "InvestorID"),
                     String::NewFromUtf8(isolate, pInputOrderAction->InvestorID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OrderActionRef"),
                     Int32::New(isolate, pInputOrderAction->OrderActionRef));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OrderRef"),
                     String::NewFromUtf8(isolate, pInputOrderAction->OrderRef));
        jsonRtn->Set(String::NewFromUtf8(isolate, "RequestID"),
                     Int32::New(isolate, pInputOrderAction->RequestID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "FrontID"),
                     Int32::New(isolate, pInputOrderAction->FrontID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "SessionID"),
                     Int32::New(isolate, pInputOrderAction->SessionID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ExchangeID"),
                     String::NewFromUtf8(isolate, pInputOrderAction->ExchangeID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OrderSysID"),
                     String::NewFromUtf8(isolate, pInputOrderAction->OrderSysID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ActionFlag"), String::NewFromUtf8(isolate,
                                                                                     charto_string(
                                                                                             pInputOrderAction->ActionFlag).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "LimitPrice"),
                     Number::New(isolate, pInputOrderAction->LimitPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate, "VolumeChange"),
                     Int32::New(isolate, pInputOrderAction->VolumeChange));
        jsonRtn->Set(String::NewFromUtf8(isolate, "UserID"),
                     String::NewFromUtf8(isolate, pInputOrderAction->UserID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "InstrumentID"),
                     String::NewFromUtf8(isolate, pInputOrderAction->InstrumentID));
        *(cbArray + 2) = jsonRtn;
    }
    else {
        *(cbArray + 2) = Local<Value>::New(isolate, Undefined(isolate));
    }
    *(cbArray + 3) = pkg_rspinfo(data->rspInfo);
    return;
}

void WrapTrader::pkg_cb_errorderaction(CbRtnField *data, Local <Value> *cbArray) {
    if (data->rtnField) {
        CThostFtdcOrderActionField *pOrderAction = static_cast<CThostFtdcOrderActionField *>(data->rtnField);
        Local <Object> jsonRtn = Object::New(isolate);
        jsonRtn->Set(String::NewFromUtf8(isolate, "BrokerID"),
                     String::NewFromUtf8(isolate, pOrderAction->BrokerID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "InvestorID"),
                     String::NewFromUtf8(isolate, pOrderAction->InvestorID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OrderActionRef"),
                     Int32::New(isolate, pOrderAction->OrderActionRef));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OrderRef"),
                     String::NewFromUtf8(isolate, pOrderAction->OrderRef));
        jsonRtn->Set(String::NewFromUtf8(isolate, "RequestID"),
                     Int32::New(isolate, pOrderAction->RequestID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "FrontID"),
                     Int32::New(isolate, pOrderAction->FrontID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "SessionID"),
                     Int32::New(isolate, pOrderAction->SessionID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ExchangeID"),
                     String::NewFromUtf8(isolate, pOrderAction->ExchangeID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OrderSysID"),
                     String::NewFromUtf8(isolate, pOrderAction->OrderSysID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ActionFlag"),
                     String::NewFromUtf8(isolate, charto_string(pOrderAction->ActionFlag).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "LimitPrice"),
                     Number::New(isolate, pOrderAction->LimitPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate, "VolumeChange"),
                     Int32::New(isolate, pOrderAction->VolumeChange));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ActionDate"),
                     String::NewFromUtf8(isolate, pOrderAction->ActionDate));
        jsonRtn->Set(String::NewFromUtf8(isolate, "TraderID"),
                     String::NewFromUtf8(isolate, pOrderAction->TraderID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "InstallID"),
                     Int32::New(isolate, pOrderAction->InstallID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OrderLocalID"),
                     String::NewFromUtf8(isolate, pOrderAction->OrderLocalID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ActionLocalID"),
                     String::NewFromUtf8(isolate, pOrderAction->ActionLocalID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ParticipantID"),
                     String::NewFromUtf8(isolate, pOrderAction->ParticipantID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ClientID"),
                     String::NewFromUtf8(isolate, pOrderAction->ClientID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "BusinessUnit"),
                     String::NewFromUtf8(isolate, pOrderAction->BusinessUnit));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OrderActionStatus"), String::NewFromUtf8(isolate,
                                                                                            charto_string(
                                                                                                    pOrderAction->OrderActionStatus).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "UserID"),
                     String::NewFromUtf8(isolate, pOrderAction->UserID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "StatusMsg"),
                     String::NewFromUtf8(isolate, pOrderAction->StatusMsg));
        jsonRtn->Set(String::NewFromUtf8(isolate, "InstrumentID"),
                     String::NewFromUtf8(isolate, pOrderAction->InstrumentID));
        *cbArray = jsonRtn;
    }
    else {
        *cbArray = Local<Value>::New(isolate, Undefined(isolate));
    }
    *(cbArray + 1) = pkg_rspinfo(data->rspInfo);
    return;
}

void WrapTrader::pkg_cb_rspqryorder(CbRtnField *data, Local <Value> *cbArray) {
    *cbArray = Int32::New(isolate, data->nRequestID);
    *(cbArray + 1) = Boolean::New(isolate, data->bIsLast)->ToBoolean();
    if (data->rtnField) {
        CThostFtdcOrderField *pOrder = static_cast<CThostFtdcOrderField *>(data->rtnField);
        Local <Object> jsonRtn = Object::New(isolate);
        jsonRtn->Set(String::NewFromUtf8(isolate, "BrokerID"),
                     String::NewFromUtf8(isolate, pOrder->BrokerID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "InvestorID"),
                     String::NewFromUtf8(isolate, pOrder->InvestorID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "InstrumentID"),
                     String::NewFromUtf8(isolate, pOrder->InstrumentID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OrderRef"),
                     String::NewFromUtf8(isolate, pOrder->OrderRef));
        jsonRtn->Set(String::NewFromUtf8(isolate, "UserID"),
                     String::NewFromUtf8(isolate, pOrder->UserID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OrderPriceType"),
                     String::NewFromUtf8(isolate, charto_string(pOrder->OrderPriceType).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "Direction"), String::NewFromUtf8(isolate,
                                                                                    charto_string(
                                                                                            pOrder->Direction).c_str()));  //var charval = String.fromCharCode(asciival);
        jsonRtn->Set(String::NewFromUtf8(isolate, "CombOffsetFlag"),
                     String::NewFromUtf8(isolate, pOrder->CombOffsetFlag));
        jsonRtn->Set(String::NewFromUtf8(isolate, "CombHedgeFlag"),
                     String::NewFromUtf8(isolate, pOrder->CombHedgeFlag));
        jsonRtn->Set(String::NewFromUtf8(isolate, "LimitPrice"),
                     Number::New(isolate, pOrder->LimitPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate, "VolumeTotalOriginal"),
                     Int32::New(isolate, pOrder->VolumeTotalOriginal));
        jsonRtn->Set(String::NewFromUtf8(isolate, "TimeCondition"),
                     String::NewFromUtf8(isolate, charto_string(pOrder->TimeCondition).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "GTDDate"),
                     String::NewFromUtf8(isolate, pOrder->GTDDate));
        jsonRtn->Set(String::NewFromUtf8(isolate, "VolumeCondition"),
                     String::NewFromUtf8(isolate, charto_string(pOrder->VolumeCondition).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "MinVolume"),
                     Int32::New(isolate, pOrder->MinVolume));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ContingentCondition"),
                     String::NewFromUtf8(isolate,
                                         charto_string(pOrder->ContingentCondition).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "StopPrice"),
                     Number::New(isolate, pOrder->StopPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ForceCloseReason"),
                     String::NewFromUtf8(isolate, charto_string(pOrder->ForceCloseReason).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "IsAutoSuspend"),
                     Int32::New(isolate, pOrder->IsAutoSuspend));
        jsonRtn->Set(String::NewFromUtf8(isolate, "BusinessUnit"),
                     String::NewFromUtf8(isolate, pOrder->BusinessUnit));
        jsonRtn->Set(String::NewFromUtf8(isolate, "RequestID"),
                     Int32::New(isolate, pOrder->RequestID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OrderLocalID"),
                     String::NewFromUtf8(isolate, pOrder->OrderLocalID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ExchangeID"),
                     String::NewFromUtf8(isolate, pOrder->ExchangeID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ParticipantID"),
                     String::NewFromUtf8(isolate, pOrder->ParticipantID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ClientID"),
                     String::NewFromUtf8(isolate, pOrder->ClientID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ExchangeInstID"),
                     String::NewFromUtf8(isolate, pOrder->ExchangeInstID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "TraderID"),
                     String::NewFromUtf8(isolate, pOrder->TraderID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "InstallID"),
                     Int32::New(isolate, pOrder->InstallID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OrderSubmitStatus"), String::NewFromUtf8(isolate,
                                                                                            charto_string(
                                                                                                    pOrder->OrderSubmitStatus).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "NotifySequence"),
                     Int32::New(isolate, pOrder->NotifySequence));
        jsonRtn->Set(String::NewFromUtf8(isolate, "TradingDay"),
                     String::NewFromUtf8(isolate, pOrder->TradingDay));
        jsonRtn->Set(String::NewFromUtf8(isolate, "SettlementID"),
                     Int32::New(isolate, pOrder->SettlementID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OrderSysID"),
                     String::NewFromUtf8(isolate, pOrder->OrderSysID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OrderSource"),
                     Int32::New(isolate, pOrder->OrderSource));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OrderStatus"),
                     String::NewFromUtf8(isolate, charto_string(pOrder->OrderStatus).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OrderType"),
                     String::NewFromUtf8(isolate, charto_string(pOrder->OrderType).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "VolumeTraded"),
                     Int32::New(isolate, pOrder->VolumeTraded));
        jsonRtn->Set(String::NewFromUtf8(isolate, "VolumeTotal"),
                     Int32::New(isolate, pOrder->VolumeTotal));
        jsonRtn->Set(String::NewFromUtf8(isolate, "InsertDate"),
                     String::NewFromUtf8(isolate, pOrder->InsertDate));
        jsonRtn->Set(String::NewFromUtf8(isolate, "InsertTime"),
                     String::NewFromUtf8(isolate, pOrder->InsertTime));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ActiveTime"),
                     String::NewFromUtf8(isolate, pOrder->ActiveTime));
        jsonRtn->Set(String::NewFromUtf8(isolate, "SuspendTime"),
                     String::NewFromUtf8(isolate, pOrder->SuspendTime));
        jsonRtn->Set(String::NewFromUtf8(isolate, "UpdateTime"),
                     String::NewFromUtf8(isolate, pOrder->UpdateTime));
        jsonRtn->Set(String::NewFromUtf8(isolate, "CancelTime"),
                     String::NewFromUtf8(isolate, pOrder->CancelTime));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ActiveTraderID"),
                     String::NewFromUtf8(isolate, pOrder->ActiveTraderID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ClearingPartID"),
                     String::NewFromUtf8(isolate, pOrder->ClearingPartID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "SequenceNo"),
                     Int32::New(isolate, pOrder->SequenceNo));
        jsonRtn->Set(String::NewFromUtf8(isolate, "FrontID"), Int32::New(isolate, pOrder->FrontID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "SessionID"),
                     Int32::New(isolate, pOrder->SessionID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "UserProductInfo"),
                     String::NewFromUtf8(isolate, pOrder->UserProductInfo));
        jsonRtn->Set(String::NewFromUtf8(isolate, "StatusMsg"),
                     String::NewFromUtf8(isolate, pOrder->StatusMsg));
        jsonRtn->Set(String::NewFromUtf8(isolate, "UserForceClose"),
                     Int32::New(isolate, pOrder->UserForceClose));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ActiveUserID"),
                     String::NewFromUtf8(isolate, pOrder->ActiveUserID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "BrokerOrderSeq"),
                     Int32::New(isolate, pOrder->BrokerOrderSeq));
        jsonRtn->Set(String::NewFromUtf8(isolate, "RelativeOrderSysID"),
                     String::NewFromUtf8(isolate, pOrder->RelativeOrderSysID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ZCETotalTradedVolume"),
                     Int32::New(isolate, pOrder->ZCETotalTradedVolume));
        jsonRtn->Set(String::NewFromUtf8(isolate, "IsSwapOrder"),
                     Int32::New(isolate, pOrder->IsSwapOrder));
        *(cbArray + 2) = jsonRtn;
    }
    else {
        *(cbArray + 2) = Local<Value>::New(isolate, Undefined(isolate));
    }
    *(cbArray + 3) = pkg_rspinfo(data->rspInfo);
    return;
}

void WrapTrader::pkg_cb_rtnorder(CbRtnField *data, Local <Value> *cbArray) {
    if (data->rtnField) {
        CThostFtdcOrderField *pOrder = static_cast<CThostFtdcOrderField *>(data->rtnField);
        Local <Object> jsonRtn = Object::New(isolate);
        jsonRtn->Set(String::NewFromUtf8(isolate, "BrokerID"),
                     String::NewFromUtf8(isolate, pOrder->BrokerID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "InvestorID"),
                     String::NewFromUtf8(isolate, pOrder->InvestorID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "InstrumentID"),
                     String::NewFromUtf8(isolate, pOrder->InstrumentID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OrderRef"),
                     String::NewFromUtf8(isolate, pOrder->OrderRef));
        jsonRtn->Set(String::NewFromUtf8(isolate, "UserID"),
                     String::NewFromUtf8(isolate, pOrder->UserID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OrderPriceType"),
                     String::NewFromUtf8(isolate, charto_string(pOrder->OrderPriceType).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "Direction"), String::NewFromUtf8(isolate,
                                                                                    charto_string(
                                                                                            pOrder->Direction).c_str()));  //var charval = String.fromCharCode(asciival);
        jsonRtn->Set(String::NewFromUtf8(isolate, "CombOffsetFlag"),
                     String::NewFromUtf8(isolate, pOrder->CombOffsetFlag));
        jsonRtn->Set(String::NewFromUtf8(isolate, "CombHedgeFlag"),
                     String::NewFromUtf8(isolate, pOrder->CombHedgeFlag));
        jsonRtn->Set(String::NewFromUtf8(isolate, "LimitPrice"),
                     Number::New(isolate, pOrder->LimitPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate, "VolumeTotalOriginal"),
                     Int32::New(isolate, pOrder->VolumeTotalOriginal));
        jsonRtn->Set(String::NewFromUtf8(isolate, "TimeCondition"),
                     String::NewFromUtf8(isolate, charto_string(pOrder->TimeCondition).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "GTDDate"),
                     String::NewFromUtf8(isolate, pOrder->GTDDate));
        jsonRtn->Set(String::NewFromUtf8(isolate, "VolumeCondition"),
                     String::NewFromUtf8(isolate, charto_string(pOrder->VolumeCondition).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "MinVolume"),
                     Int32::New(isolate, pOrder->MinVolume));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ContingentCondition"),
                     String::NewFromUtf8(isolate,
                                         charto_string(pOrder->ContingentCondition).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "StopPrice"),
                     Number::New(isolate, pOrder->StopPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ForceCloseReason"),
                     String::NewFromUtf8(isolate, charto_string(pOrder->ForceCloseReason).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "IsAutoSuspend"),
                     Int32::New(isolate, pOrder->IsAutoSuspend));
        jsonRtn->Set(String::NewFromUtf8(isolate, "BusinessUnit"),
                     String::NewFromUtf8(isolate, pOrder->BusinessUnit));
        jsonRtn->Set(String::NewFromUtf8(isolate, "RequestID"),
                     Int32::New(isolate, pOrder->RequestID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OrderLocalID"),
                     String::NewFromUtf8(isolate, pOrder->OrderLocalID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ExchangeID"),
                     String::NewFromUtf8(isolate, pOrder->ExchangeID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ParticipantID"),
                     String::NewFromUtf8(isolate, pOrder->ParticipantID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ClientID"),
                     String::NewFromUtf8(isolate, pOrder->ClientID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ExchangeInstID"),
                     String::NewFromUtf8(isolate, pOrder->ExchangeInstID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "TraderID"),
                     String::NewFromUtf8(isolate, pOrder->TraderID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "InstallID"),
                     Int32::New(isolate, pOrder->InstallID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OrderSubmitStatus"), String::NewFromUtf8(isolate,
                                                                                            charto_string(
                                                                                                    pOrder->OrderSubmitStatus).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "NotifySequence"),
                     Int32::New(isolate, pOrder->NotifySequence));
        jsonRtn->Set(String::NewFromUtf8(isolate, "TradingDay"),
                     String::NewFromUtf8(isolate, pOrder->TradingDay));
        jsonRtn->Set(String::NewFromUtf8(isolate, "SettlementID"),
                     Int32::New(isolate, pOrder->SettlementID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OrderSysID"),
                     String::NewFromUtf8(isolate, pOrder->OrderSysID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OrderSource"),
                     Int32::New(isolate, pOrder->OrderSource));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OrderStatus"),
                     String::NewFromUtf8(isolate, charto_string(pOrder->OrderStatus).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OrderType"),
                     String::NewFromUtf8(isolate, charto_string(pOrder->OrderType).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "VolumeTraded"),
                     Int32::New(isolate, pOrder->VolumeTraded));
        jsonRtn->Set(String::NewFromUtf8(isolate, "VolumeTotal"),
                     Int32::New(isolate, pOrder->VolumeTotal));
        jsonRtn->Set(String::NewFromUtf8(isolate, "InsertDate"),
                     String::NewFromUtf8(isolate, pOrder->InsertDate));
        jsonRtn->Set(String::NewFromUtf8(isolate, "InsertTime"),
                     String::NewFromUtf8(isolate, pOrder->InsertTime));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ActiveTime"),
                     String::NewFromUtf8(isolate, pOrder->ActiveTime));
        jsonRtn->Set(String::NewFromUtf8(isolate, "SuspendTime"),
                     String::NewFromUtf8(isolate, pOrder->SuspendTime));
        jsonRtn->Set(String::NewFromUtf8(isolate, "UpdateTime"),
                     String::NewFromUtf8(isolate, pOrder->UpdateTime));
        jsonRtn->Set(String::NewFromUtf8(isolate, "CancelTime"),
                     String::NewFromUtf8(isolate, pOrder->CancelTime));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ActiveTraderID"),
                     String::NewFromUtf8(isolate, pOrder->ActiveTraderID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ClearingPartID"),
                     String::NewFromUtf8(isolate, pOrder->ClearingPartID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "SequenceNo"),
                     Int32::New(isolate, pOrder->SequenceNo));
        jsonRtn->Set(String::NewFromUtf8(isolate, "FrontID"), Int32::New(isolate, pOrder->FrontID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "SessionID"),
                     Int32::New(isolate, pOrder->SessionID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "UserProductInfo"),
                     String::NewFromUtf8(isolate, pOrder->UserProductInfo));
        jsonRtn->Set(String::NewFromUtf8(isolate, "StatusMsg"),
                     String::NewFromUtf8(isolate, pOrder->StatusMsg));
        jsonRtn->Set(String::NewFromUtf8(isolate, "UserForceClose"),
                     Int32::New(isolate, pOrder->UserForceClose));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ActiveUserID"),
                     String::NewFromUtf8(isolate, pOrder->ActiveUserID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "BrokerOrderSeq"),
                     Int32::New(isolate, pOrder->BrokerOrderSeq));
        jsonRtn->Set(String::NewFromUtf8(isolate, "RelativeOrderSysID"),
                     String::NewFromUtf8(isolate, pOrder->RelativeOrderSysID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ZCETotalTradedVolume"),
                     Int32::New(isolate, pOrder->ZCETotalTradedVolume));
        jsonRtn->Set(String::NewFromUtf8(isolate, "IsSwapOrder"),
                     Int32::New(isolate, pOrder->IsSwapOrder));
        *cbArray = jsonRtn;
    }
    else {
        *cbArray = Local<Value>::New(isolate, Undefined(isolate));
    }
    return;
}

void WrapTrader::pkg_cb_rqtrade(CbRtnField *data, Local <Value> *cbArray) {
    *cbArray = Int32::New(isolate, data->nRequestID);
    *(cbArray + 1) = Boolean::New(isolate, data->bIsLast)->ToBoolean();
    if (data->rtnField) {
        CThostFtdcTradeField *pTrade = static_cast<CThostFtdcTradeField *>(data->rtnField);
        Local <Object> jsonRtn = Object::New(isolate);
        jsonRtn->Set(String::NewFromUtf8(isolate, "BrokerID"),
                     String::NewFromUtf8(isolate, pTrade->BrokerID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "InvestorID"),
                     String::NewFromUtf8(isolate, pTrade->InvestorID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "InstrumentID"),
                     String::NewFromUtf8(isolate, pTrade->InstrumentID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OrderRef"),
                     String::NewFromUtf8(isolate, pTrade->OrderRef));
        jsonRtn->Set(String::NewFromUtf8(isolate, "UserID"),
                     String::NewFromUtf8(isolate, pTrade->UserID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ExchangeID"),
                     String::NewFromUtf8(isolate, pTrade->ExchangeID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "TradeID"),
                     String::NewFromUtf8(isolate, pTrade->TradeID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "Direction"), String::NewFromUtf8(isolate,
                                                                                    charto_string(
                                                                                            pTrade->Direction).c_str()));  //var charval = String.fromCharCode(asciival);
        jsonRtn->Set(String::NewFromUtf8(isolate, "OrderSysID"),
                     String::NewFromUtf8(isolate, pTrade->OrderSysID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ParticipantID"),
                     String::NewFromUtf8(isolate, pTrade->ParticipantID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ClientID"),
                     String::NewFromUtf8(isolate, pTrade->ClientID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "TradingRole"),
                     String::NewFromUtf8(isolate, charto_string(pTrade->TradingRole).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ExchangeInstID"),
                     String::NewFromUtf8(isolate, pTrade->ExchangeInstID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OffsetFlag"),
                     String::NewFromUtf8(isolate, charto_string(pTrade->OffsetFlag).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "HedgeFlag"),
                     String::NewFromUtf8(isolate, charto_string(pTrade->HedgeFlag).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "Price"), Number::New(isolate, pTrade->Price));
        jsonRtn->Set(String::NewFromUtf8(isolate, "Volume"), Int32::New(isolate, pTrade->Volume));
        jsonRtn->Set(String::NewFromUtf8(isolate, "TradeDate"),
                     String::NewFromUtf8(isolate, pTrade->TradeDate));
        jsonRtn->Set(String::NewFromUtf8(isolate, "TradeTime"),
                     String::NewFromUtf8(isolate, pTrade->TradeTime));
        jsonRtn->Set(String::NewFromUtf8(isolate, "TradeType"),
                     String::NewFromUtf8(isolate, charto_string(pTrade->TradeType).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "PriceSource"),
                     String::NewFromUtf8(isolate, charto_string(pTrade->PriceSource).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "TraderID"),
                     String::NewFromUtf8(isolate, pTrade->TraderID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OrderLocalID"),
                     String::NewFromUtf8(isolate, pTrade->OrderLocalID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ClearingPartID"),
                     String::NewFromUtf8(isolate, pTrade->ClearingPartID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "BusinessUnit"),
                     String::NewFromUtf8(isolate, pTrade->BusinessUnit));
        jsonRtn->Set(String::NewFromUtf8(isolate, "SequenceNo"),
                     Int32::New(isolate, pTrade->SequenceNo));
        jsonRtn->Set(String::NewFromUtf8(isolate, "TradingDay"),
                     String::NewFromUtf8(isolate, pTrade->TradingDay));
        jsonRtn->Set(String::NewFromUtf8(isolate, "SettlementID"),
                     Int32::New(isolate, pTrade->SettlementID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "BrokerOrderSeq"),
                     Int32::New(isolate, pTrade->BrokerOrderSeq));
        jsonRtn->Set(String::NewFromUtf8(isolate, "TradeSource"),
                     String::NewFromUtf8(isolate, charto_string(pTrade->TradeSource).c_str()));
        *(cbArray + 2) = jsonRtn;
    }
    else {
        *(cbArray + 2) = Local<Value>::New(isolate, Undefined(isolate));
    }
    *(cbArray + 3) = pkg_rspinfo(data->rspInfo);
    return;
}

void WrapTrader::pkg_cb_rtntrade(CbRtnField *data, Local <Value> *cbArray) {
    if (data->rtnField) {
        CThostFtdcTradeField *pTrade = static_cast<CThostFtdcTradeField *>(data->rtnField);
        Local <Object> jsonRtn = Object::New(isolate);
        jsonRtn->Set(String::NewFromUtf8(isolate, "BrokerID"),
                     String::NewFromUtf8(isolate, pTrade->BrokerID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "InvestorID"),
                     String::NewFromUtf8(isolate, pTrade->InvestorID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "InstrumentID"),
                     String::NewFromUtf8(isolate, pTrade->InstrumentID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OrderRef"),
                     String::NewFromUtf8(isolate, pTrade->OrderRef));
        jsonRtn->Set(String::NewFromUtf8(isolate, "UserID"),
                     String::NewFromUtf8(isolate, pTrade->UserID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ExchangeID"),
                     String::NewFromUtf8(isolate, pTrade->ExchangeID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "TradeID"),
                     String::NewFromUtf8(isolate, pTrade->TradeID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "Direction"), String::NewFromUtf8(isolate,
                                                                                    charto_string(
                                                                                            pTrade->Direction).c_str()));  //var charval = String.fromCharCode(asciival);
        jsonRtn->Set(String::NewFromUtf8(isolate, "OrderSysID"),
                     String::NewFromUtf8(isolate, pTrade->OrderSysID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ParticipantID"),
                     String::NewFromUtf8(isolate, pTrade->ParticipantID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ClientID"),
                     String::NewFromUtf8(isolate, pTrade->ClientID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "TradingRole"),
                     Int32::New(isolate, pTrade->TradingRole));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ExchangeInstID"),
                     String::NewFromUtf8(isolate, pTrade->ExchangeInstID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OffsetFlag"),
                     Int32::New(isolate, pTrade->OffsetFlag));
        jsonRtn->Set(String::NewFromUtf8(isolate, "HedgeFlag"),
                     Int32::New(isolate, pTrade->HedgeFlag));
        jsonRtn->Set(String::NewFromUtf8(isolate, "Price"), Number::New(isolate, pTrade->Price));
        jsonRtn->Set(String::NewFromUtf8(isolate, "Volume"), Int32::New(isolate, pTrade->Volume));
        jsonRtn->Set(String::NewFromUtf8(isolate, "TradeDate"),
                     String::NewFromUtf8(isolate, pTrade->TradeDate));
        jsonRtn->Set(String::NewFromUtf8(isolate, "TradeTime"),
                     String::NewFromUtf8(isolate, pTrade->TradeTime));
        jsonRtn->Set(String::NewFromUtf8(isolate, "TradeType"),
                     Int32::New(isolate, pTrade->TradeType));
        jsonRtn->Set(String::NewFromUtf8(isolate, "PriceSource"),
                     String::NewFromUtf8(isolate, charto_string(pTrade->PriceSource).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "TraderID"),
                     String::NewFromUtf8(isolate, pTrade->TraderID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OrderLocalID"),
                     String::NewFromUtf8(isolate, pTrade->OrderLocalID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ClearingPartID"),
                     String::NewFromUtf8(isolate, pTrade->ClearingPartID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "BusinessUnit"),
                     String::NewFromUtf8(isolate, pTrade->BusinessUnit));
        jsonRtn->Set(String::NewFromUtf8(isolate, "SequenceNo"),
                     Int32::New(isolate, pTrade->SequenceNo));
        jsonRtn->Set(String::NewFromUtf8(isolate, "TradingDay"),
                     String::NewFromUtf8(isolate, pTrade->TradingDay));
        jsonRtn->Set(String::NewFromUtf8(isolate, "SettlementID"),
                     Int32::New(isolate, pTrade->SettlementID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "BrokerOrderSeq"),
                     Int32::New(isolate, pTrade->BrokerOrderSeq));
        jsonRtn->Set(String::NewFromUtf8(isolate, "TradeSource"),
                     String::NewFromUtf8(isolate, charto_string(pTrade->TradeSource).c_str()));
        *cbArray = jsonRtn;
    }
    else {
        *cbArray = Local<Value>::New(isolate, Undefined(isolate));
    }

    return;
}

void WrapTrader::pkg_cb_rqinvestorposition(CbRtnField *data, Local <Value> *cbArray) {
    *cbArray = Int32::New(isolate, data->nRequestID);
    *(cbArray + 1) = Boolean::New(isolate, data->bIsLast)->ToBoolean();
    if (data->rtnField) {
        CThostFtdcInvestorPositionField *_pInvestorPosition = static_cast<CThostFtdcInvestorPositionField *>(data->rtnField);
        Local <Object> jsonRtn = Object::New(isolate);
        jsonRtn->Set(String::NewFromUtf8(isolate, "InstrumentID"),
                     String::NewFromUtf8(isolate, _pInvestorPosition->InstrumentID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "BrokerID"),
                     String::NewFromUtf8(isolate, _pInvestorPosition->BrokerID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "InvestorID"),
                     String::NewFromUtf8(isolate, _pInvestorPosition->InvestorID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "PosiDirection"), String::NewFromUtf8(isolate,
                                                                                        charto_string(
                                                                                                _pInvestorPosition->PosiDirection).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "HedgeFlag"), String::NewFromUtf8(isolate,
                                                                                    charto_string(
                                                                                            _pInvestorPosition->HedgeFlag).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "PositionDate"),
                     Int32::New(isolate, _pInvestorPosition->PositionDate));
        jsonRtn->Set(String::NewFromUtf8(isolate, "YdPosition"),
                     Int32::New(isolate, _pInvestorPosition->YdPosition));
        jsonRtn->Set(String::NewFromUtf8(isolate, "Position"),
                     Int32::New(isolate, _pInvestorPosition->Position));
        jsonRtn->Set(String::NewFromUtf8(isolate, "LongFrozen"),
                     Int32::New(isolate, _pInvestorPosition->LongFrozen));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ShortFrozen"),
                     Int32::New(isolate, _pInvestorPosition->ShortFrozen));
        jsonRtn->Set(String::NewFromUtf8(isolate, "LongFrozenAmount"),
                     Number::New(isolate, _pInvestorPosition->LongFrozenAmount));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ShortFrozenAmount"),
                     Number::New(isolate, _pInvestorPosition->ShortFrozenAmount));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OpenVolume"),
                     Int32::New(isolate, _pInvestorPosition->OpenVolume));
        jsonRtn->Set(String::NewFromUtf8(isolate, "CloseVolume"),
                     Int32::New(isolate, _pInvestorPosition->CloseVolume));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OpenAmount"),
                     Number::New(isolate, _pInvestorPosition->OpenAmount));
        jsonRtn->Set(String::NewFromUtf8(isolate, "CloseAmount"),
                     Number::New(isolate, _pInvestorPosition->CloseAmount));
        jsonRtn->Set(String::NewFromUtf8(isolate, "PositionCost"),
                     Number::New(isolate, _pInvestorPosition->PositionCost));
        jsonRtn->Set(String::NewFromUtf8(isolate, "PreMargin"),
                     Number::New(isolate, _pInvestorPosition->PreMargin));
        jsonRtn->Set(String::NewFromUtf8(isolate, "UseMargin"),
                     Number::New(isolate, _pInvestorPosition->UseMargin));
        jsonRtn->Set(String::NewFromUtf8(isolate, "FrozenMargin"),
                     Number::New(isolate, _pInvestorPosition->FrozenMargin));
        jsonRtn->Set(String::NewFromUtf8(isolate, "FrozenCash"),
                     Number::New(isolate, _pInvestorPosition->FrozenCash));
        jsonRtn->Set(String::NewFromUtf8(isolate, "FrozenCommission"),
                     Number::New(isolate, _pInvestorPosition->FrozenCommission));
        jsonRtn->Set(String::NewFromUtf8(isolate, "CashIn"),
                     Number::New(isolate, _pInvestorPosition->CashIn));
        jsonRtn->Set(String::NewFromUtf8(isolate, "Commission"),
                     Number::New(isolate, _pInvestorPosition->Commission));
        jsonRtn->Set(String::NewFromUtf8(isolate, "CloseProfit"),
                     Number::New(isolate, _pInvestorPosition->CloseProfit));
        jsonRtn->Set(String::NewFromUtf8(isolate, "PositionProfit"),
                     Number::New(isolate, _pInvestorPosition->PositionProfit));
        jsonRtn->Set(String::NewFromUtf8(isolate, "PreSettlementPrice"),
                     Number::New(isolate, _pInvestorPosition->PreSettlementPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate, "SettlementPrice"),
                     Number::New(isolate, _pInvestorPosition->SettlementPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate, "TradingDay"),
                     String::NewFromUtf8(isolate, _pInvestorPosition->TradingDay));
        jsonRtn->Set(String::NewFromUtf8(isolate, "SettlementID"),
                     Int32::New(isolate, _pInvestorPosition->SettlementID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OpenCost"),
                     Number::New(isolate, _pInvestorPosition->OpenCost));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ExchangeMargin"),
                     Number::New(isolate, _pInvestorPosition->ExchangeMargin));
        jsonRtn->Set(String::NewFromUtf8(isolate, "CombPosition"),
                     Int32::New(isolate, _pInvestorPosition->CombPosition));
        jsonRtn->Set(String::NewFromUtf8(isolate, "CombLongFrozen"),
                     Int32::New(isolate, _pInvestorPosition->CombLongFrozen));
        jsonRtn->Set(String::NewFromUtf8(isolate, "CombShortFrozen"),
                     Int32::New(isolate, _pInvestorPosition->CombShortFrozen));
        jsonRtn->Set(String::NewFromUtf8(isolate, "CloseProfitByDate"),
                     Number::New(isolate, _pInvestorPosition->CloseProfitByDate));
        jsonRtn->Set(String::NewFromUtf8(isolate, "CloseProfitByTrade"),
                     Number::New(isolate, _pInvestorPosition->CloseProfitByTrade));
        jsonRtn->Set(String::NewFromUtf8(isolate, "TodayPosition"),
                     Int32::New(isolate, _pInvestorPosition->TodayPosition));
        jsonRtn->Set(String::NewFromUtf8(isolate, "MarginRateByMoney"),
                     Number::New(isolate, _pInvestorPosition->MarginRateByMoney));
        jsonRtn->Set(String::NewFromUtf8(isolate, "MarginRateByVolume"),
                     Number::New(isolate, _pInvestorPosition->MarginRateByVolume));
        *(cbArray + 2) = jsonRtn;
    }
    else {
        *(cbArray + 2) = Local<Value>::New(isolate, Undefined(isolate));
    }
    *(cbArray + 3) = pkg_rspinfo(data->rspInfo);
    return;
}

void WrapTrader::pkg_cb_rqinvestorpositiondetail(CbRtnField *data, Local <Value> *cbArray) {
    *cbArray = Int32::New(isolate, data->nRequestID);
    *(cbArray + 1) = Boolean::New(isolate, data->bIsLast)->ToBoolean();
    if (data->rtnField) {
        CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail = static_cast<CThostFtdcInvestorPositionDetailField *>(data->rtnField);
        Local <Object> jsonRtn = Object::New(isolate);
        jsonRtn->Set(String::NewFromUtf8(isolate, "InstrumentID"),
                     String::NewFromUtf8(isolate, pInvestorPositionDetail->InstrumentID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "BrokerID"),
                     String::NewFromUtf8(isolate, pInvestorPositionDetail->BrokerID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "InvestorID"),
                     String::NewFromUtf8(isolate, pInvestorPositionDetail->InvestorID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "HedgeFlag"), String::NewFromUtf8(isolate,
                                                                                    charto_string(
                                                                                            pInvestorPositionDetail->HedgeFlag).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "Direction"), String::NewFromUtf8(isolate,
                                                                                    charto_string(
                                                                                            pInvestorPositionDetail->Direction).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OpenDate"),
                     String::NewFromUtf8(isolate, pInvestorPositionDetail->OpenDate));
        jsonRtn->Set(String::NewFromUtf8(isolate, "TradeID"),
                     String::NewFromUtf8(isolate, pInvestorPositionDetail->TradeID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "Volume"),
                     Int32::New(isolate, pInvestorPositionDetail->Volume));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OpenPrice"),
                     Number::New(isolate, pInvestorPositionDetail->OpenPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate, "TradingDay"),
                     String::NewFromUtf8(isolate, pInvestorPositionDetail->TradingDay));
        jsonRtn->Set(String::NewFromUtf8(isolate, "SettlementID"),
                     Int32::New(isolate, pInvestorPositionDetail->SettlementID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "TradeType"), String::NewFromUtf8(isolate,
                                                                                    charto_string(
                                                                                            pInvestorPositionDetail->TradeType).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "CombInstrumentID"),
                     String::NewFromUtf8(isolate, pInvestorPositionDetail->CombInstrumentID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ExchangeID"),
                     String::NewFromUtf8(isolate, pInvestorPositionDetail->ExchangeID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "CloseProfitByDate"),
                     Number::New(isolate, pInvestorPositionDetail->CloseProfitByDate));
        jsonRtn->Set(String::NewFromUtf8(isolate, "CloseProfitByTrade"),
                     Number::New(isolate, pInvestorPositionDetail->CloseProfitByTrade));
        jsonRtn->Set(String::NewFromUtf8(isolate, "PositionProfitByDate"),
                     Number::New(isolate, pInvestorPositionDetail->PositionProfitByDate));
        jsonRtn->Set(String::NewFromUtf8(isolate, "PositionProfitByTrade"),
                     Number::New(isolate, pInvestorPositionDetail->PositionProfitByTrade));
        jsonRtn->Set(String::NewFromUtf8(isolate, "Margin"),
                     Number::New(isolate, pInvestorPositionDetail->Margin));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ExchMargin"),
                     Number::New(isolate, pInvestorPositionDetail->ExchMargin));
        jsonRtn->Set(String::NewFromUtf8(isolate, "MarginRateByMoney"),
                     Number::New(isolate, pInvestorPositionDetail->MarginRateByMoney));
        jsonRtn->Set(String::NewFromUtf8(isolate, "MarginRateByVolume"),
                     Number::New(isolate, pInvestorPositionDetail->MarginRateByVolume));
        jsonRtn->Set(String::NewFromUtf8(isolate, "LastSettlementPrice"),
                     Number::New(isolate, pInvestorPositionDetail->LastSettlementPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate, "SettlementPrice"),
                     Number::New(isolate, pInvestorPositionDetail->SettlementPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate, "CloseVolume"),
                     Int32::New(isolate, pInvestorPositionDetail->CloseVolume));
        jsonRtn->Set(String::NewFromUtf8(isolate, "CloseAmount"),
                     Number::New(isolate, pInvestorPositionDetail->CloseAmount));
        *(cbArray + 2) = jsonRtn;
    }
    else {
        *(cbArray + 2) = Local<Value>::New(isolate, Undefined(isolate));
    }
    *(cbArray + 3) = pkg_rspinfo(data->rspInfo);
    return;
}

void WrapTrader::pkg_cb_rqtradingaccount(CbRtnField *data, Local <Value> *cbArray) {
    *cbArray = Int32::New(isolate, data->nRequestID);
    *(cbArray + 1) = Boolean::New(isolate, data->bIsLast)->ToBoolean();
    if (data->rtnField) {
        CThostFtdcTradingAccountField *pTradingAccount = static_cast<CThostFtdcTradingAccountField *>(data->rtnField);
        Local <Object> jsonRtn = Object::New(isolate);
        jsonRtn->Set(String::NewFromUtf8(isolate, "BrokerID"),
                     String::NewFromUtf8(isolate, pTradingAccount->BrokerID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "AccountID"),
                     String::NewFromUtf8(isolate, pTradingAccount->AccountID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "PreMortgage"),
                     Number::New(isolate, pTradingAccount->PreMortgage));
        jsonRtn->Set(String::NewFromUtf8(isolate, "PreCredit"),
                     Number::New(isolate, pTradingAccount->PreCredit));
        jsonRtn->Set(String::NewFromUtf8(isolate, "PreDeposit"),
                     Number::New(isolate, pTradingAccount->PreDeposit));
        jsonRtn->Set(String::NewFromUtf8(isolate, "PreBalance"),
                     Number::New(isolate, pTradingAccount->PreBalance));
        jsonRtn->Set(String::NewFromUtf8(isolate, "PreMargin"),
                     Number::New(isolate, pTradingAccount->PreMargin));
        jsonRtn->Set(String::NewFromUtf8(isolate, "InterestBase"),
                     Number::New(isolate, pTradingAccount->InterestBase));
        jsonRtn->Set(String::NewFromUtf8(isolate, "Interest"),
                     Number::New(isolate, pTradingAccount->Interest));
        jsonRtn->Set(String::NewFromUtf8(isolate, "Deposit"),
                     Number::New(isolate, pTradingAccount->Deposit));
        jsonRtn->Set(String::NewFromUtf8(isolate, "Withdraw"),
                     Number::New(isolate, pTradingAccount->Withdraw));
        jsonRtn->Set(String::NewFromUtf8(isolate, "FrozenMargin"),
                     Number::New(isolate, pTradingAccount->FrozenMargin));
        jsonRtn->Set(String::NewFromUtf8(isolate, "FrozenCash"),
                     Number::New(isolate, pTradingAccount->FrozenCash));
        jsonRtn->Set(String::NewFromUtf8(isolate, "FrozenCommission"),
                     Number::New(isolate, pTradingAccount->FrozenCommission));
        jsonRtn->Set(String::NewFromUtf8(isolate, "CurrMargin"),
                     Number::New(isolate, pTradingAccount->CurrMargin));
        jsonRtn->Set(String::NewFromUtf8(isolate, "CashIn"),
                     Number::New(isolate, pTradingAccount->CashIn));
        jsonRtn->Set(String::NewFromUtf8(isolate, "Commission"),
                     Number::New(isolate, pTradingAccount->Commission));
        jsonRtn->Set(String::NewFromUtf8(isolate, "CloseProfit"),
                     Number::New(isolate, pTradingAccount->CloseProfit));
        jsonRtn->Set(String::NewFromUtf8(isolate, "PositionProfit"),
                     Number::New(isolate, pTradingAccount->PositionProfit));
        jsonRtn->Set(String::NewFromUtf8(isolate, "Balance"),
                     Number::New(isolate, pTradingAccount->Balance));
        jsonRtn->Set(String::NewFromUtf8(isolate, "Available"),
                     Number::New(isolate, pTradingAccount->Available));
        jsonRtn->Set(String::NewFromUtf8(isolate, "WithdrawQuota"),
                     Number::New(isolate, pTradingAccount->WithdrawQuota));
        jsonRtn->Set(String::NewFromUtf8(isolate, "Reserve"),
                     Number::New(isolate, pTradingAccount->Reserve));
        jsonRtn->Set(String::NewFromUtf8(isolate, "TradingDay"),
                     String::NewFromUtf8(isolate, pTradingAccount->TradingDay));
        jsonRtn->Set(String::NewFromUtf8(isolate, "SettlementID"),
                     Int32::New(isolate, pTradingAccount->SettlementID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "Credit"),
                     Number::New(isolate, pTradingAccount->Credit));
        jsonRtn->Set(String::NewFromUtf8(isolate, "Mortgage"),
                     Number::New(isolate, pTradingAccount->Mortgage));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ExchangeMargin"),
                     Number::New(isolate, pTradingAccount->ExchangeMargin));
        jsonRtn->Set(String::NewFromUtf8(isolate, "DeliveryMargin"),
                     Number::New(isolate, pTradingAccount->DeliveryMargin));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ExchangeDeliveryMargin"),
                     Number::New(isolate, pTradingAccount->ExchangeDeliveryMargin));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ReserveBalance"),
                     Number::New(isolate, pTradingAccount->ReserveBalance));
        jsonRtn->Set(String::NewFromUtf8(isolate, "CurrencyID"),
                     String::NewFromUtf8(isolate, pTradingAccount->CurrencyID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "PreFundMortgageIn"),
                     Number::New(isolate, pTradingAccount->PreFundMortgageIn));
        jsonRtn->Set(String::NewFromUtf8(isolate, "PreFundMortgageOut"),
                     Number::New(isolate, pTradingAccount->PreFundMortgageOut));
        jsonRtn->Set(String::NewFromUtf8(isolate, "FundMortgageIn"),
                     Number::New(isolate, pTradingAccount->FundMortgageIn));
        jsonRtn->Set(String::NewFromUtf8(isolate, "FundMortgageOut"),
                     Number::New(isolate, pTradingAccount->FundMortgageOut));
        jsonRtn->Set(String::NewFromUtf8(isolate, "FundMortgageAvailable"),
                     Number::New(isolate, pTradingAccount->FundMortgageAvailable));
        jsonRtn->Set(String::NewFromUtf8(isolate, "MortgageableFund"),
                     Number::New(isolate, pTradingAccount->MortgageableFund));
        jsonRtn->Set(String::NewFromUtf8(isolate, "SpecProductMargin"),
                     Number::New(isolate, pTradingAccount->SpecProductMargin));
        jsonRtn->Set(String::NewFromUtf8(isolate, "SpecProductFrozenMargin"),
                     Number::New(isolate, pTradingAccount->SpecProductFrozenMargin));
        jsonRtn->Set(String::NewFromUtf8(isolate, "SpecProductCommission"),
                     Number::New(isolate, pTradingAccount->SpecProductCommission));
        jsonRtn->Set(String::NewFromUtf8(isolate, "SpecProductFrozenCommission"),
                     Number::New(isolate, pTradingAccount->SpecProductFrozenCommission));
        jsonRtn->Set(String::NewFromUtf8(isolate, "SpecProductPositionProfit"),
                     Number::New(isolate, pTradingAccount->SpecProductPositionProfit));
        jsonRtn->Set(String::NewFromUtf8(isolate, "SpecProductCloseProfit"),
                     Number::New(isolate, pTradingAccount->SpecProductCloseProfit));
        jsonRtn->Set(String::NewFromUtf8(isolate, "SpecProductPositionProfitByAlg"),
                     Number::New(isolate, pTradingAccount->SpecProductPositionProfitByAlg));
        jsonRtn->Set(String::NewFromUtf8(isolate, "SpecProductExchangeMargin"),
                     Number::New(isolate, pTradingAccount->SpecProductExchangeMargin));
        *(cbArray + 2) = jsonRtn;
    }
    else {
        *(cbArray + 2) = Local<Value>::New(isolate, Undefined(isolate));
    }
    *(cbArray + 3) = pkg_rspinfo(data->rspInfo);
    return;
}

void WrapTrader::pkg_cb_rqinstrument(CbRtnField *data, Local <Value> *cbArray) {
    *cbArray = Int32::New(isolate, data->nRequestID);
    *(cbArray + 1) = Boolean::New(isolate, data->bIsLast)->ToBoolean();
    if (data->rtnField) {
        CThostFtdcInstrumentField *pInstrument = static_cast<CThostFtdcInstrumentField *>(data->rtnField);
        Local <Object> jsonRtn = Object::New(isolate);
        jsonRtn->Set(String::NewFromUtf8(isolate, "InstrumentID"),
                     String::NewFromUtf8(isolate, pInstrument->InstrumentID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ExchangeID"),
                     String::NewFromUtf8(isolate, pInstrument->ExchangeID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "InstrumentName"),
                     String::NewFromUtf8(isolate, pInstrument->InstrumentName));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ExchangeInstID"),
                     String::NewFromUtf8(isolate, pInstrument->ExchangeInstID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ProductID"),
                     String::NewFromUtf8(isolate, pInstrument->ProductID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ProductClass"), String::NewFromUtf8(isolate,
                                                                                       charto_string(
                                                                                               pInstrument->ProductClass).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "DeliveryYear"),
                     Int32::New(isolate, pInstrument->DeliveryYear));
        jsonRtn->Set(String::NewFromUtf8(isolate, "DeliveryMonth"),
                     Int32::New(isolate, pInstrument->DeliveryMonth));
        jsonRtn->Set(String::NewFromUtf8(isolate, "MaxMarketOrderVolume"),
                     Int32::New(isolate, pInstrument->MaxMarketOrderVolume));
        jsonRtn->Set(String::NewFromUtf8(isolate, "MinMarketOrderVolume"),
                     Int32::New(isolate, pInstrument->MinMarketOrderVolume));
        jsonRtn->Set(String::NewFromUtf8(isolate, "MaxLimitOrderVolume"),
                     Int32::New(isolate, pInstrument->MaxLimitOrderVolume));
        jsonRtn->Set(String::NewFromUtf8(isolate, "MinLimitOrderVolume"),
                     Int32::New(isolate, pInstrument->MinLimitOrderVolume));
        jsonRtn->Set(String::NewFromUtf8(isolate, "VolumeMultiple"),
                     Int32::New(isolate, pInstrument->VolumeMultiple));
        jsonRtn->Set(String::NewFromUtf8(isolate, "PriceTick"),
                     Number::New(isolate, pInstrument->PriceTick));
        jsonRtn->Set(String::NewFromUtf8(isolate, "CreateDate"),
                     String::NewFromUtf8(isolate, pInstrument->CreateDate));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OpenDate"),
                     String::NewFromUtf8(isolate, pInstrument->OpenDate));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ExpireDate"),
                     String::NewFromUtf8(isolate, pInstrument->ExpireDate));
        jsonRtn->Set(String::NewFromUtf8(isolate, "StartDelivDate"),
                     String::NewFromUtf8(isolate, pInstrument->StartDelivDate));
        jsonRtn->Set(String::NewFromUtf8(isolate, "EndDelivDate"),
                     String::NewFromUtf8(isolate, pInstrument->EndDelivDate));
        jsonRtn->Set(String::NewFromUtf8(isolate, "InstLifePhase"),
                     Int32::New(isolate, pInstrument->InstLifePhase));
        jsonRtn->Set(String::NewFromUtf8(isolate, "IsTrading"),
                     Int32::New(isolate, pInstrument->IsTrading));
        jsonRtn->Set(String::NewFromUtf8(isolate, "PositionType"), String::NewFromUtf8(isolate,
                                                                                       charto_string(
                                                                                               pInstrument->PositionType).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "PositionDateType"), String::NewFromUtf8(isolate,
                                                                                           charto_string(
                                                                                                   pInstrument->PositionDateType).c_str()));
        jsonRtn->Set(String::NewFromUtf8(isolate, "LongMarginRatio"),
                     Number::New(isolate, pInstrument->LongMarginRatio));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ShortMarginRatio"),
                     Number::New(isolate, pInstrument->ShortMarginRatio));
        jsonRtn->Set(String::NewFromUtf8(isolate, "MaxMarginSideAlgorithm"),
                     String::NewFromUtf8(isolate, charto_string(
                             pInstrument->MaxMarginSideAlgorithm).c_str()));
        *(cbArray + 2) = jsonRtn;
    }
    else {
        *(cbArray + 2) = Local<Value>::New(isolate, Undefined(isolate));
    }
    *(cbArray + 3) = pkg_rspinfo(data->rspInfo);
    return;
}

void WrapTrader::pkg_cb_rqdepthmarketdata(CbRtnField *data, Local <Value> *cbArray) {
    *cbArray = Int32::New(isolate, data->nRequestID);
    *(cbArray + 1) = Boolean::New(isolate, data->bIsLast)->ToBoolean();
    if (data->rtnField) {
        CThostFtdcDepthMarketDataField *pDepthMarketData = static_cast<CThostFtdcDepthMarketDataField *>(data->rtnField);
        Local <Object> jsonRtn = Object::New(isolate);
        jsonRtn->Set(String::NewFromUtf8(isolate, "TradingDay"),
                     String::NewFromUtf8(isolate, pDepthMarketData->TradingDay));
        jsonRtn->Set(String::NewFromUtf8(isolate, "InstrumentID"),
                     String::NewFromUtf8(isolate, pDepthMarketData->InstrumentID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ExchangeID"),
                     String::NewFromUtf8(isolate, pDepthMarketData->ExchangeID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ExchangeInstID"),
                     String::NewFromUtf8(isolate, pDepthMarketData->ExchangeInstID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "LastPrice"),
                     Number::New(isolate, pDepthMarketData->LastPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate, "PreSettlementPrice"),
                     Number::New(isolate, pDepthMarketData->PreSettlementPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate, "PreClosePrice"),
                     Number::New(isolate, pDepthMarketData->PreClosePrice));
        jsonRtn->Set(String::NewFromUtf8(isolate, "PreOpenInterest"),
                     Number::New(isolate, pDepthMarketData->PreOpenInterest));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OpenPrice"),
                     Number::New(isolate, pDepthMarketData->OpenPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate, "HighestPrice"),
                     Number::New(isolate, pDepthMarketData->HighestPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate, "LowestPrice"),
                     Number::New(isolate, pDepthMarketData->LowestPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate, "Volume"),
                     Int32::New(isolate, pDepthMarketData->Volume));
        jsonRtn->Set(String::NewFromUtf8(isolate, "Turnover"),
                     Number::New(isolate, pDepthMarketData->Turnover));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OpenInterest"),
                     Number::New(isolate, pDepthMarketData->OpenInterest));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ClosePrice"),
                     Number::New(isolate, pDepthMarketData->ClosePrice));
        jsonRtn->Set(String::NewFromUtf8(isolate, "SettlementPrice"),
                     Number::New(isolate, pDepthMarketData->SettlementPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate, "UpperLimitPrice"),
                     Number::New(isolate, pDepthMarketData->UpperLimitPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate, "LowerLimitPrice"),
                     Number::New(isolate, pDepthMarketData->LowerLimitPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate, "PreDelta"),
                     Number::New(isolate, pDepthMarketData->PreDelta));
        jsonRtn->Set(String::NewFromUtf8(isolate, "CurrDelta"),
                     Number::New(isolate, pDepthMarketData->CurrDelta));
        jsonRtn->Set(String::NewFromUtf8(isolate, "UpdateTime"),
                     String::NewFromUtf8(isolate, pDepthMarketData->UpdateTime));
        jsonRtn->Set(String::NewFromUtf8(isolate, "UpdateMillisec"),
                     Int32::New(isolate, pDepthMarketData->UpdateMillisec));
        jsonRtn->Set(String::NewFromUtf8(isolate, "BidPrice1"),
                     Number::New(isolate, pDepthMarketData->BidPrice1));
        jsonRtn->Set(String::NewFromUtf8(isolate, "BidVolume1"),
                     Number::New(isolate, pDepthMarketData->BidVolume1));
        jsonRtn->Set(String::NewFromUtf8(isolate, "AskPrice1"),
                     Number::New(isolate, pDepthMarketData->AskPrice1));
        jsonRtn->Set(String::NewFromUtf8(isolate, "AskVolume1"),
                     Number::New(isolate, pDepthMarketData->AskVolume1));
        jsonRtn->Set(String::NewFromUtf8(isolate, "BidPrice2"),
                     Number::New(isolate, pDepthMarketData->BidPrice2));
        jsonRtn->Set(String::NewFromUtf8(isolate, "BidVolume2"),
                     Number::New(isolate, pDepthMarketData->BidVolume2));
        jsonRtn->Set(String::NewFromUtf8(isolate, "AskPrice2"),
                     Number::New(isolate, pDepthMarketData->AskPrice2));
        jsonRtn->Set(String::NewFromUtf8(isolate, "AskVolume2"),
                     Number::New(isolate, pDepthMarketData->AskVolume2));
        jsonRtn->Set(String::NewFromUtf8(isolate, "BidPrice3"),
                     Number::New(isolate, pDepthMarketData->BidPrice3));
        jsonRtn->Set(String::NewFromUtf8(isolate, "BidVolume3"),
                     Number::New(isolate, pDepthMarketData->BidVolume3));
        jsonRtn->Set(String::NewFromUtf8(isolate, "AskPrice3"),
                     Number::New(isolate, pDepthMarketData->AskPrice3));
        jsonRtn->Set(String::NewFromUtf8(isolate, "AskVolume3"),
                     Number::New(isolate, pDepthMarketData->AskVolume3));
        jsonRtn->Set(String::NewFromUtf8(isolate, "BidPrice4"),
                     Number::New(isolate, pDepthMarketData->BidPrice4));
        jsonRtn->Set(String::NewFromUtf8(isolate, "BidVolume4"),
                     Number::New(isolate, pDepthMarketData->BidVolume4));
        jsonRtn->Set(String::NewFromUtf8(isolate, "AskPrice4"),
                     Number::New(isolate, pDepthMarketData->AskPrice4));
        jsonRtn->Set(String::NewFromUtf8(isolate, "AskVolume4"),
                     Number::New(isolate, pDepthMarketData->AskVolume4));
        jsonRtn->Set(String::NewFromUtf8(isolate, "BidPrice5"),
                     Number::New(isolate, pDepthMarketData->BidPrice5));
        jsonRtn->Set(String::NewFromUtf8(isolate, "BidVolume5"),
                     Number::New(isolate, pDepthMarketData->BidVolume5));
        jsonRtn->Set(String::NewFromUtf8(isolate, "AskPrice5"),
                     Number::New(isolate, pDepthMarketData->AskPrice5));
        jsonRtn->Set(String::NewFromUtf8(isolate, "AskVolume5"),
                     Number::New(isolate, pDepthMarketData->AskVolume5));
        jsonRtn->Set(String::NewFromUtf8(isolate, "AveragePrice"),
                     Number::New(isolate, pDepthMarketData->AveragePrice));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ActionDay"),
                     String::NewFromUtf8(isolate, pDepthMarketData->ActionDay));
        *(cbArray + 2) = jsonRtn;
    }
    else {
        *(cbArray + 2) = Local<Value>::New(isolate, Undefined(isolate));
    }
    *(cbArray + 3) = pkg_rspinfo(data->rspInfo);
    return;
}

void WrapTrader::pkg_cb_rqsettlementinfo(CbRtnField *data, Local <Value> *cbArray) {
    *cbArray = Int32::New(isolate, data->nRequestID);
    *(cbArray + 1) = Boolean::New(isolate, data->bIsLast)->ToBoolean();
    if (data->rtnField != NULL) {
        CThostFtdcSettlementInfoField *pSettlementInfo = static_cast<CThostFtdcSettlementInfoField *>(data->rtnField);
        Local <Object> jsonRtn = Object::New(isolate);
        jsonRtn->Set(String::NewFromUtf8(isolate, "TradingDay"),
                     String::NewFromUtf8(isolate, pSettlementInfo->TradingDay));
        jsonRtn->Set(String::NewFromUtf8(isolate, "SettlementID"),
                     Int32::New(isolate, pSettlementInfo->SettlementID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "BrokerID"),
                     String::NewFromUtf8(isolate, pSettlementInfo->BrokerID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "InvestorID"),
                     String::NewFromUtf8(isolate, pSettlementInfo->InvestorID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "SequenceNo"),
                     Int32::New(isolate, pSettlementInfo->SequenceNo));
        jsonRtn->Set(String::NewFromUtf8(isolate, "Content"),
                     String::NewFromUtf8(isolate, pSettlementInfo->Content));
        *(cbArray + 2) = jsonRtn;
    }
    else {
        *(cbArray + 2) = Local<Value>::New(isolate, Undefined(isolate));
    }
    *(cbArray + 3) = pkg_rspinfo(data->rspInfo);
    return;
}

void WrapTrader::pkg_cb_rsperror(CbRtnField *data, Local <Value> *cbArray) {
    *cbArray = Int32::New(isolate, data->nRequestID);
    *(cbArray + 1) = Boolean::New(isolate, data->bIsLast)->ToBoolean();
    *(cbArray + 2) = pkg_rspinfo(data->rspInfo);
    return;
}

Local <Value> WrapTrader::pkg_rspinfo(void *vpRspInfo) {
    if (vpRspInfo) {
        CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField *>(vpRspInfo);
        Local <Object> jsonInfo = Object::New(isolate);
        jsonInfo->Set(String::NewFromUtf8(isolate, "ErrorID"),
                      Int32::New(isolate, pRspInfo->ErrorID));
        jsonInfo->Set(String::NewFromUtf8(isolate, "ErrorMsg"),
                      String::NewFromUtf8(isolate, pRspInfo->ErrorMsg));
        return jsonInfo;
    }
    else {
        return Local<Value>::New(isolate, Undefined(isolate));
    }
}
