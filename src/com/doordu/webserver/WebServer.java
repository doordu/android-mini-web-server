package com.doordu.webserver;

public class WebServer {
	
	/**
	 * 初始化方法
	 * @param cert_path 证书目录
	 * @param port 监听端口
	 * @return 1
	 */
	public native boolean Init(String cert_path, String port);
	
	/**
	 * 设置Token
	 * @param token 开门密钥
	 */
	public native void SetToken(String token);
	
	/**
	 * 执行
	 * @return
	 */
	public native int Start();
	
	/**
	 * 预留暂时没用
	 */
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
