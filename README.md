# Android Mini Web Server

## 编译

    nkd-build
    
    
## 使用

* 将编译好的libs目录下动态库libwebserver.so导入到工程目录libs下
* 将com.doordu.webserver.WebServer拷贝到src目录下
* 使用如下方式加载

        static {
        	System.loadLibrary("webserver");
        }
    
* 使用如下方式执行

        WebServer s = new WebServer();
        s.Init("/data/data/cert.pem", "8081"); // 设置证书文件和监听端口
        s.SetToken("123456"); // 设置Token
        s.Start();    
        
* 修改callback调用开门指令

        package com.doordu.webserver;
        
        public class WebServer {
        	public native boolean Init(String cert_path, String port);
        	public native void SetToken(String token);
        	public native int Start();
        	public native void Destory();
        	
        	/**
        	 * token验证成功后回调
        	 * @param roomNo 房号
        	 * @return 开门成功返回1, 失败返回0
        	 */
        	public int callback(String roomNo) {
        		System.out.println(String.format("Open door! %s", roomNo));
        		return 1;
        	}
        }
        

    
    