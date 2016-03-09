package com.qualcomm.ar.pl;

import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.IOException;
import org.apache.http.Header;
import org.apache.http.HttpEntity;
import org.apache.http.HttpResponse;
import org.apache.http.StatusLine;

public class ARHttpResponse
{
    private static final String MODULENAME = "ARHttpResponse";
    public static int ERROR_NONE = 0;
    public static int ERROR_CANCELED = 1;
    public static int ERROR_OPERATION_FAILED = 2;
    public static int ERROR_TIMEOUT = 3;
    public int statusCode;
    public int networkStatus;
    public String contentType;
    public String contentEncoding;
    public byte[] contentBytes;

    public static ARHttpResponse createARResponse(int errorReason)
    {
        if ((errorReason > 0) && (errorReason < 4))
        {
            ARHttpResponse arResponse = new ARHttpResponse();
            arResponse.statusCode = 0;
            arResponse.contentBytes = null;
            arResponse.contentEncoding = null;
            arResponse.contentType = null;

            arResponse.networkStatus = errorReason;
            return arResponse;
        }

        return null;
    }

    public static ARHttpResponse createARResponse(HttpResponse httpResponse) throws IOException
    {
        ARHttpResponse arResponse = new ARHttpResponse();

        StatusLine status = httpResponse.getStatusLine();
        arResponse.statusCode = status.getStatusCode();
        arResponse.networkStatus = ERROR_NONE;
        Header header;
        if ((header = httpResponse.getFirstHeader("Content-Type")) != null)
        {
            arResponse.contentType = header.getValue();
        }

        if ((header = httpResponse.getFirstHeader("Content-Encoding")) != null)
        {
            arResponse.contentEncoding = header.getValue();
        }

        HttpEntity entity = httpResponse.getEntity();
        if ((entity != null) && (entity.getContentLength() > 0L))
        {
            arResponse.contentBytes = new byte[(int)entity.getContentLength()];

            DataInputStream is = new DataInputStream(entity.getContent());
            is.readFully(arResponse.contentBytes);
        }
        else if (entity != null)
        {
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            DataInputStream dis = new DataInputStream(entity.getContent());
            byte[] xfer = new byte[2048];
            int bytesRead = 0;
            while ((bytesRead = dis.read(xfer)) != -1) {
                baos.write(xfer, 0, bytesRead);
            }
            arResponse.contentBytes = baos.toByteArray();
        }

        return arResponse;
    }
}
