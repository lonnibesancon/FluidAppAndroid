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


public class LoggingElement {

	private long timestamp ;
	private double precision ;
	private short interactionMode ;
	private short interactionType ;
	private short phase ;
	private boolean isConstrained ;
	private boolean isXConstrained ;
	private boolean isYConstrained ;
	private boolean isZConstrained ;
	private boolean isAutoConstrained ;

	public LoggingElement(long timestamp,double precison, short interactionMode,short interactionType, 
						  short phase, boolean isConstrained, boolean isXConstrained, 
						  boolean isYConstrained, boolean isZConstrained, boolean isAutoConstrained){

		this.timestamp 			= timestamp ;
		this.precision 			= precision ;
		this.interactionMode	= interactionMode ;
		this.interactionType	= interactionType ;
		this.phase 				= phase ;
		this.isConstrained 		= isConstrained ;
		this.isXConstrained 	= isXConstrained ;
		this.isYConstrained 	= isYConstrained ;
		this.isZConstrained 	= isZConstrained ;
		this.isAutoConstrained 	= isAutoConstrained ;

	}

	public String toString(){
		return (""+this.timestamp+";"
				 +this.precision+";"
				 +this.interactionMode+";"
				 +this.interactionType+";"
				 +this.phase+";"
				 +this.isConstrained+";"
				 +this.isXConstrained+";"
				 +this.isYConstrained+";"
				 +this.isZConstrained+";"
				 +this.isAutoConstrained+";"
				 +"\n") ;
	}



}