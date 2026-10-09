/*
 * WAVE Audio Playback for GNU-Linux systems.
 * Version 4.0 (Interop Java version 1.0)
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Font;

public class Definitions
{
	public static final String MAINWND_CAPTION = "WAVE Audio Playback";
	public static final Dimension MAINWND_INITSIZE = new Dimension(1280, 720);

	public static final Color MYSCREEN_BACKGROUNDCOLOR = Color.LIGHT_GRAY;

	public static final Color TEXT_FOREGROUNDCOLOR = Color.BLACK;

	public static final Color BUTTON_FOREGROUNDCOLOR = Color.BLACK;
	public static final Color BUTTON_BACKGROUNDCOLOR = Color.WHITE;

	public static final String FONT_NAME = Font.SANS_SERIF;
	public static final int FONT_STYLE = Font.PLAIN;

	public static final int INTERCOMPONENT_MARGIN = 10;
	public static final int BOTTOM_WINDOW_MARGIN = 60;

	public static final int TITLE_FONTSIZE = 30;
	public static final int TITLE_FONTSIZEMARGIN = 10;
	public static final int TITLE_MARGINLEFT = 10;
	public static final int TITLE_MARGINTOP = 10;

	public static final Font TITLE_FONT = new Font(FONT_NAME, FONT_STYLE, TITLE_FONTSIZE);
}

