package fr.limsi.ARViewer;

public interface InteractionMode{

	public static final int nothing						= 0 ;

	// Interaction mode for data
	public static final int dataTangible 				= 1 ;
	public static final int dataTouch 					= 2 ;
	//public static final int dataHybrid 					= 3 ;


	//Interaction mode for plane
	public static final int planeTouch 					= 11 ;
	public static final int planeTangible 				= 12 ;
	//public static final int planeHybrid 				= 13 ;

	// Interaction mode for plane + data
	public static final int dataPlaneTouch 				= 21 ;
	public static final int dataPlaneTangible 			= 22 ;
	public static final int dataPlaneHybrid 			= 23 ;
	public static final int dataTouchTangible 			= 24 ;
	public static final int planeTouchTangible 			= 25 ;
	public static final int dataPlaneTouchTangible		= 26 ;
	public static final int dataPlaneTangibleTouch		= 27 ;

	//Seeding point interaction
	public static final int seedPointTangible 			= 31 ;
	public static final int seedPointTouch 				= 32 ;
	public static final int seedPointHybrid 			= 33 ;


	public static final short touchInteraction 			= 1 ;
	public static final short tangibleInteraction 		= 2 ;


	public static final float thresholdRST 				= 450 ;

	public static final int ftle						= 0 ;
	public static final int head						= 1 ;
	public static final int ironprot					= 2 ;
	public static final int velocities					= 3 ;

	//LOGGING HELP



}