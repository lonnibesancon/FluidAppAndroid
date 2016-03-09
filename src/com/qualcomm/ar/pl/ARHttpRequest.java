package com.qualcomm.ar.pl;

import java.io.File;
import java.io.UnsupportedEncodingException;
import org.apache.http.HttpEntity;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.client.methods.HttpUriRequest;
import org.apache.http.entity.ByteArrayEntity;
import org.apache.http.entity.FileEntity;
import org.apache.http.entity.StringEntity;

import android.util.Log;

public class ARHttpRequest
{
    private static final String MODULENAME = "ARHttpRequest";
    public String url;
    public String method;
    public String[] headers;
    boolean isQuery;
    public String contentType = null;
    public String contentEncoding = null;
    public String contentPath = null;

    public byte[] content = null;
    public long nativeRequestPtr;

    private static final String TAG = "ARViewer";

    public static HttpUriRequest createHttpRequest(ARHttpRequest arRequest)
        throws UnsupportedEncodingException
    {
        Log.d(TAG, "[QCAR] New HTTP request");

        if (arRequest.method.equals("POST"))
        {
            Log.d(TAG, "[QCAR] POST " + arRequest.url);
            HttpPost httpPost = new HttpPost(arRequest.url);

            for (int i = 0; i < arRequest.headers.length / 2; i++)
            {
                Log.d(TAG, "[QCAR] header " + arRequest.headers[(2 * i)] + " = " + arRequest.headers[(2 * i + 1)]);
                httpPost.addHeader(arRequest.headers[(2 * i)], arRequest.headers[(2 * i + 1)]);
            }

            if (!arRequest.isQuery)
            {
                if (arRequest.contentPath != null)
                {
                    Log.d(TAG, "[QCAR] file " + arRequest.contentPath);
                    HttpEntity entity = new FileEntity(new File(arRequest.contentPath), arRequest.contentType);
                    httpPost.setEntity(entity);
                }
                else if (arRequest.content != null)
                {
                    Log.d(TAG, "[QCAR] content " + arRequest.content);
                    ByteArrayEntity bae = new ByteArrayEntity(arRequest.content);
                    bae.setContentType(arRequest.contentType);

                    HttpEntity entity = bae;
                    httpPost.setEntity(entity);
                }

            }
            else
            {
                Log.d(TAG, "[QCAR] (empty content)");
                HttpEntity entity = new StringEntity("", "UTF-8");
                httpPost.setEntity(entity);
            }

            return httpPost;
        }
        if (arRequest.method.equals("GET"))
        {
            Log.d(TAG, "[QCAR] GET " + arRequest.url);
            HttpGet httpGet = new HttpGet(arRequest.url);

            for (int i = 0; i < arRequest.headers.length / 2; i++)
            {
                Log.d(TAG, "[QCAR] header " + arRequest.headers[(2 * i)] + " = " + arRequest.headers[(2 * i + 1)]);
                httpGet.addHeader(arRequest.headers[(2 * i)], arRequest.headers[(2 * i + 1)]);
            }

            return httpGet;
        }

        throw new UnsupportedOperationException("Attemped to use an unsupported HTTP operation");
    }
}
