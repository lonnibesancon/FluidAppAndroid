package com.qualcomm.ar.pl;

import java.util.concurrent.Callable;
import java.util.concurrent.CancellationException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;
import org.apache.http.HttpResponse;
import org.apache.http.client.HttpClient;
import org.apache.http.client.methods.HttpUriRequest;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.params.HttpParams;

import android.util.Log;

public class ARHttpClient
{
    private static final String MODULENAME = "ARHttpClient";
    private Future<HttpResponse> requestTask = null;
    private Future<?> postDSNTimeoutTask = null;
    ExecutorService executor;

    private static final String TAG = "ARViewer";

    public boolean execute(ARHttpRequest arRequest, ARHttpParams arParams)
    {
        if (arRequest == null) {
            return false;
        }

        try
        {
            // this.executor = Executors.newFixedThreadPool(2);
            // this.requestTask = this.executor.submit(new HttpResponseCallable(arRequest, arParams));
            // this.postDSNTimeoutTask = this.executor.submit(new HttpResponseWatcher(arRequest, arParams));

            // // Create the request, in order to log it
            // ARHttpRequest.createHttpRequest(arRequest);

            // But do not send it
            Log.d(TAG, "[QCAR] HTTP request blocked: " + arRequest.url);

            // And return a fake response to the native code
            ARHttpResponse response = new ARHttpResponse();
            response.statusCode = 200;
            response.contentBytes = null;
            response.contentEncoding = null;
            response.contentType = null;
            nativeCallback(response, arRequest.nativeRequestPtr);
        }
        catch (Exception e)
        {
            e.printStackTrace();
            return false;
        }

        return true;
    }

    public boolean cancel()
    {
        if (this.requestTask != null)
        {
            this.requestTask.cancel(true);
            return this.requestTask.isDone();
        }

        return false;
    }

    public native void nativeCallback(ARHttpResponse paramARHttpResponse, long paramLong);

    private class HttpResponseWatcher
        implements Runnable
    {
        private ARHttpParams arParams;
        private ARHttpRequest arRequest;

        public HttpResponseWatcher(ARHttpRequest arRequest, ARHttpParams arParams)
        {
            this.arParams = arParams;
            this.arRequest = arRequest;
        }

        public void run()
        {
            ARHttpResponse arResponse = null;
            int networkStatus = ARHttpResponse.ERROR_NONE;
            try
            {
                HttpResponse httpResponse = ARHttpClient.this.requestTask.get(this.arParams.delayedStart_ms + this.arParams.requestTimeout_ms, TimeUnit.MILLISECONDS);

                arResponse = ARHttpResponse.createARResponse(httpResponse);
                if ((arResponse.statusCode == 200) || (
                        (ARHttpClient.this.requestTask != null) && (!ARHttpClient.this.requestTask.isCancelled())))
                {
                    ARHttpClient.this.requestTask.cancel(true);
                }

                if (networkStatus != ARHttpResponse.ERROR_NONE)
                    arResponse = ARHttpResponse.createARResponse(networkStatus);
            }
            catch (CancellationException e)
            {
                networkStatus = ARHttpResponse.ERROR_CANCELED;

                if ((ARHttpClient.this.requestTask != null) && (!ARHttpClient.this.requestTask.isCancelled()))
                {
                    ARHttpClient.this.requestTask.cancel(true);
                }

                if (networkStatus != ARHttpResponse.ERROR_NONE)
                    arResponse = ARHttpResponse.createARResponse(networkStatus);
            }
            catch (TimeoutException e)
            {
                networkStatus = ARHttpResponse.ERROR_TIMEOUT;

                if ((ARHttpClient.this.requestTask != null) && (!ARHttpClient.this.requestTask.isCancelled()))
                {
                    ARHttpClient.this.requestTask.cancel(true);
                }

                if (networkStatus != ARHttpResponse.ERROR_NONE)
                    arResponse = ARHttpResponse.createARResponse(networkStatus);
            }
            catch (Exception e)
            {
                networkStatus = ARHttpResponse.ERROR_OPERATION_FAILED;

                if ((ARHttpClient.this.requestTask != null) && (!ARHttpClient.this.requestTask.isCancelled()))
                {
                    ARHttpClient.this.requestTask.cancel(true);
                }

                if (networkStatus != ARHttpResponse.ERROR_NONE)
                    arResponse = ARHttpResponse.createARResponse(networkStatus);
            }
            finally
            {
                if ((ARHttpClient.this.requestTask != null) && (!ARHttpClient.this.requestTask.isCancelled()))
                {
                    ARHttpClient.this.requestTask.cancel(true);
                }

                if (networkStatus != ARHttpResponse.ERROR_NONE) {
                    arResponse = ARHttpResponse.createARResponse(networkStatus);
                }
            }
            ARHttpClient.this.nativeCallback(arResponse, this.arRequest.nativeRequestPtr);

            ARHttpClient.this.executor.shutdownNow();
        }
    }

    private class HttpResponseCallable
        implements Callable<HttpResponse>
    {
        private ARHttpRequest arRequest;
        private ARHttpParams arParams;

        public HttpResponseCallable(ARHttpRequest arRequest, ARHttpParams arParams)
        {
            this.arRequest = arRequest;
            this.arParams = arParams;
        }

        public HttpResponse call() throws Exception
        {
            if (this.arParams.delayedStart_ms != 0)
            {
                Thread.sleep(this.arParams.delayedStart_ms);
            }

            HttpParams httpParams = ARHttpParams.createHttpParams(this.arParams);

            HttpClient httpClient = new DefaultHttpClient(httpParams);

            HttpUriRequest httpPost = ARHttpRequest.createHttpRequest(this.arRequest);

            return httpClient.execute(httpPost);
        }
    }
}
