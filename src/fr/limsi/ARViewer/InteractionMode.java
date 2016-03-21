package fr.limsi.ARViewer;

public interface InteractionMode{

	// Interaction mode for data
	public static final int dataTangible 		= 1 ;
	public static final int dataTouch 			= 2 ;
	public static final int dataHybrid 			= 3 ;


	//Interaction mode for plane
	public static final int planeTouch 			= 11 ;
	public static final int planeTangible 		= 12 ;
	public static final int planeHybrid 		= 13 ;

	// Interaction mode for plane + data
	public static final int dataPlaneTouch 		= 21 ;
	public static final int dataPlaneTangible 	= 22 ;
	public static final int dataPlaneHybrid 	= 23 ;

	//Seeding point interaction
	public static final int seedPointTangible = 31 ;
	public static final int seedPointTouch = 32 ;
	public static final int seedPointHybrid = 33 ;


	public static final short touchInteraction = 1 ;
	public static final short tangibleInteraction = 2 ;


	public static final float thresholdRST = 450 ;


	//LOGGING HELP



}