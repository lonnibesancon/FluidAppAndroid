package fr.limsi.ARViewer;

import java.io.*;
import java.net.*;
import java.lang.*;
import java.util.*;
import java.lang.Object;
import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.text.DecimalFormat ;
import android.util.Log;


public class Logging {

	private ArrayList<LoggingElement> logging ;
	protected int size ;

	public Logging(){
		logging = new ArrayList<LoggingElement>();
		size = 0 ;
	}


	public void addLog(long timestamp,double precison, short interactionMode, short interactionType, 
					   short phase,boolean isConstrained, boolean isXConstrained, boolean isYConstrained,
				       boolean isZConstrained, boolean isAutoConstrained){

		LoggingElement tmp = new LoggingElement(timestamp,precison,interactionMode,interactionType,phase,
				   								isConstrained,isXConstrained,isYConstrained,
				   								isZConstrained,isAutoConstrained);

		logging.add(tmp);
		Log.d("COUCOU",""+size);
		size ++ ;
	}

	public String getString(int i){
		return logging.get(i).toString();
	}


	/*(long timestamp,double precison, short interactionMode, short phase,
				   boolean isConstrained, boolean isXConstrained, boolean isYConstrained,
				   boolean isZConstrained, boolean isAutoConstrained){

		this.timestamp 			= timestamp ;
		this.precision 			= precision ;
		this.interactionMode	= interactionMode ;
		this.phase 				= phase ;
		this.isConstrained 		= isConstrained ;
		this.isXConstrained 	= isXConstrained ;
		this.isYConstrained 	= isYConstrained ;
		this.isZConstrained 	= isZConstrained ;
		this.isAutoConstrained 	= isAutoConstrained ;

	}*/

	/*public String toString(){
		return (""+this.timestamp+";"
				 +this.precision+";"
				 +this.interactionMode+";"
				 +this.phase+";"
				 +this.isConstrained+";"
				 +this.isXConstrained+";"
				 +this.isYConstrained+";"
				 +this.isZConstrained+";"
				 +this.isAutoConstrained+";") ;
	}*/



}