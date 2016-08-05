#include <jni.h>
#include <android/log.h>

#include "com_doordu_webserver_WebServer.h"
#include "mongoose.h"


#define LOG_TAG    "com.doordu.webserver"
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

char *s_http_port = "";
char *s_cert_path = "";
char *s_token = "";
JavaVM *gvm = NULL;
jobject jobj;

static struct mg_serve_http_opts s_http_server_opts;


JNIEnv* getJNIEnv(int* needsDetach){
    JNIEnv* env = NULL;
    if ((*gvm)->GetEnv(gvm, (void**) &env, JNI_VERSION_1_4) != JNI_OK){
        int status = (*gvm)->AttachCurrentThread(gvm, &env, 0);
        if (status < 0){
            LOGE("failed to attach current thread");
            return NULL;
        }
        *needsDetach = 1;
    }
    LOGI("GetEnv Success");
    return env;
}

static void handle_open_door(struct mg_connection *nc, struct http_message *hm) {
	struct mg_str *doordu_hdr = mg_get_http_header(hm, "Doordu");

	/* Send headers */
	if (doordu_hdr != NULL && mg_vcmp(doordu_hdr, s_token) == 0) {
		JNIEnv *env;
		jclass jcls;
		jmethodID jmid;
		char room_no[100];
		int needsDetach;

		env = getJNIEnv(&needsDetach);
		jcls = (*env)->GetObjectClass(env, jobj);
		if (jcls == NULL) {
			LOGE("FindClass() Error.....");
			return;
		}

		jmid = (*env)->GetMethodID(env, jcls, "callback", "(Ljava/lang/String;)I");
		if (jmid == NULL) {
			LOGE("GetMethodID() Error.....");
			return;
		}

		mg_get_http_var(&hm->query_string, "room_id", room_no, sizeof(room_no));

		//mg_printf_http_chunk(nc, "{ \"result\": 1 }\n");
		jstring jstr = (*env)->NewStringUTF(env, room_no);

		jint status = (*env)->CallIntMethod(env, jobj, jmid, jstr);
		if (status) {
			mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
			mg_printf_http_chunk(nc, "{\"message\": \"开门成功\",\"data\": []}\n");
		} else {
			mg_printf(nc, "%s", "HTTP/1.1 404 Not Found\r\nTransfer-Encoding: chunked\r\n\r\n");
		}
	    if(needsDetach) {
//	    	(*gvm)->DetachCurrentThread(gvm);
	    }
	} else {
		//mg_printf_http_chunk(nc, "{ \"result\": 0 }\n");
		mg_printf(nc, "%s", "HTTP/1.1 401 Authorization Required\r\nTransfer-Encoding: chunked\r\n\r\n");
	}
	mg_send_http_chunk(nc, "", 0); /* Send empty chunk, the end of response */
}

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
	struct http_message *hm = (struct http_message *) ev_data;
	if (ev == MG_EV_HTTP_REQUEST) {
		LOGI("requested...");
		if (mg_vcmp(&hm->uri, "/v3/doors/open") == 0) {
				handle_open_door(nc, hm);
		}
		mg_serve_http(nc, (struct http_message *) ev_data, s_http_server_opts);
	}
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = NULL;
    jint result = -1;

    // 获取JNI版本
    if ((*vm)->GetEnv(vm, (void**)&env, JNI_VERSION_1_4) != JNI_OK) {
    	LOGE("GETEnv failed!");
    	return result;
    }

    return JNI_VERSION_1_4;
}

/*
 * Class:     com_doordu_webserver_WebServer
 * Method:    Init
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_doordu_webserver_WebServer_Init
  (JNIEnv *env, jobject obj, jstring cert_path, jstring port) {
	s_cert_path = (*env)->GetStringUTFChars(env, cert_path, 0);
	s_http_port = (*env)->GetStringUTFChars(env, port, 0);
	return 1;
}

/*
 * Class:     com_doordu_webserver_WebServer
 * Method:    SetToken
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_doordu_webserver_WebServer_SetToken
  (JNIEnv *env, jobject obj, jstring token) {
	s_token = (*env)->GetStringUTFChars(env, token, 0);
}


/*
 * Class:     com_doordu_webserver_WebServer
 * Method:    Start
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_doordu_webserver_WebServer_Start
  (JNIEnv *env, jobject obj) {
	struct mg_mgr mgr;
	struct mg_connection *nc;


	(*env)->GetJavaVM(env, &gvm);
	jobj = (*env)->NewGlobalRef(env, obj);

	mg_mgr_init(&mgr, NULL);
	nc = mg_bind(&mgr, s_http_port, ev_handler);

	const char *err_str = mg_set_ssl(nc, s_cert_path, NULL);
	if (err_str != NULL) {
		LOGE("Error loading SSL cert: %s\n", err_str);
		exit(1);
	}

	// Set up HTTP server parameters
	mg_set_protocol_http_websocket(nc);
	s_http_server_opts.document_root = ".";      // Serve current directory
	s_http_server_opts.dav_document_root = ".";  // Allow access via WebDav
	s_http_server_opts.enable_directory_listing = "no";

	LOGI("Starting web server on port %s\n", s_http_port);
	for (;;) {
		mg_mgr_poll(&mgr, 1000);
	}
	mg_mgr_free(&mgr);

	return 0;
}

/*
 * Class:     com_doordu_webserver_WebServer
 * Method:    Destory
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_doordu_webserver_WebServer_Destory
  (JNIEnv *env, jobject obj) {

}


