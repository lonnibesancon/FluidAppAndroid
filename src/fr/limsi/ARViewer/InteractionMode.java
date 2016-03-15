package fr.limsi.ARViewer;

public interface InteractionMode{
	public static final int dataTangibleOnly = 1 ;
	public static final int dataTouchOnly = 2 ;
	//Touch for rotation, Tangible for translation
	public static final int dataTouchTangible = 3 ;
	public static final int dataTangibleTouch = 4 ;



	public static final int sliceTangibleOnly = 11 ;
	public static final int sliceTouchOnly = 12 ;
	public static final int sliceTouchTangible = 13 ;
	public static final int sliceTangibleTouch = 14 ;



	public static final int dataSliceTouchTangible = 21 ;
	public static final int dataSliceTangibleTouch = 22 ;



	public static final int seedPoint = 31 ;


	public static final short touchInteraction = 1 ;
	public static final short tangibleInteraction = 2 ;


	//LOGGING HELP



}