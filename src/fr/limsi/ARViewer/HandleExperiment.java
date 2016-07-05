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


public class HandleExperiment implements InteractionMode {

	protected int participantId ;
	protected ArrayList<Integer> order ;
	protected int currentMode ;

	public HandleExperiment(int id){
		this.participantId = id ;
		order = new ArrayList<Integer>();
		/*switch(id){
			case 1:
				order.add(dataTangible);
				order.add(dataTouch);
				order.add(dataHybrid);
				order.add(planeTangible);
				order.add(planeTouch);
				order.add(planeHybrid);
				order.add(dataPlaneTangible);
				order.add(dataPlaneTouch);
				order.add(dataPlaneHybrid);
				order.add(seedPoint);
				break;

			case 2:
				order.add(dataTangible);
				order.add(dataHybrid);
				order.add(dataTouch);
				order.add(planeTangible);
				order.add(planeHybrid);
				order.add(planeTouch);
				order.add(dataPlaneTangible);
				order.add(dataPlaneHybrid);
				order.add(dataPlaneTouch);
				order.add(seedPoint);
				break;

			case 3:
				order.add(dataTouch);
				order.add(dataTangible);
				order.add(dataHybrid);
				order.add(planeTouch);
				order.add(planeTangible);
				order.add(planeHybrid);
				order.add(dataPlaneTouch);
				order.add(dataPlaneTangible);
				order.add(dataPlaneHybrid);
				order.add(seedPoint);
				break;

			case 4:
				order.add(dataTouch);
				order.add(dataHybrid);
				order.add(dataTangible);
				order.add(planeTouch);
				order.add(planeHybrid);
				order.add(planeTangible);
				order.add(dataPlaneTouch);
				order.add(dataPlaneHybrid);
				order.add(dataPlaneTangible);
				order.add(seedPoint);
				break;

			case 5:
				order.add(dataHybrid);
				order.add(dataTouch);
				order.add(dataTangible);
				order.add(planeHybrid);
				order.add(planeTouch);
				order.add(planeTangible);
				order.add(dataPlaneHybrid);
				order.add(dataPlaneTouch);
				order.add(dataPlaneTangible);
				order.add(seedPoint);
				break;

			case 6:
				order.add(dataHybrid);
				order.add(dataTangible);
				order.add(dataTouch);
				order.add(planeHybrid);
				order.add(planeTangible);
				order.add(planeTouch);
				order.add(dataPlaneHybrid);
				order.add(dataPlaneTangible);
				order.add(dataPlaneTouch);
				order.add(seedPoint);
				break;

		}*/
	}

	public int getInteractionMode(){
		currentMode = order.get(0);
		order.remove(0);
		return currentMode ;
	}

	/*public String getFileName(){
		String s ="";
		switch(currentMode){
			case dataTangible:
				s="dataTangible";
				break;
			case dataTouch:
				s="dataTouch";
				break;
			case dataHybrid:
				s="dataHybrid";
				break;
			case planeTangible:
				s="planeTangible";
				break;
			case planeTouch:
				s="planeTouch";
				break;
			case planeHybrid:
				s="planeHybrid";
				break;
			case dataPlaneTangible:
				s="dataPlaneTangible";
				break;
			case dataPlaneTouch:
				s="dataPlaneTouch";
				break;
			case dataPlaneHybrid:
				s="dataPlaneHybrid";
				break;
			case seedPointTangible:
				s="seedPoint";
				break;
			case seedPointTouch:
				s="seedPoint";
				break;
			case seedPointHybrid:
				s="seedPoint";
				break;
		}
		return ("Participant_"+participantId+"-"+s) ;
	}*/

}