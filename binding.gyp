{
  "targets": [
    {
      "target_name": "shifctp",
      "sources": [ "src/shifctp.cc","src/tools.cc","src/stdafx.cpp","src/uv_mduser.cpp","src/uv_trader.cpp","src/wrap_mduser.cpp","src/wrap_trader.cpp" ],
      "libraries":["/home/zhangls/code/self/node_ctp/6.3.0_20140811_traderapi_linux64/thostmduserapi.so","/home/zhangls/code/self/node_ctp/6.3.0_20140811_traderapi_linux64/thosttraderapi.so"],
      "include_dirs":["6.3.0_20140811_traderapi_linux64/"]
    }
  ],
}


