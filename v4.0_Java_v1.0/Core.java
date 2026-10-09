/*
 * WAVE Audio Playback for GNU-Linux systems.
 * Version 4.0 (Interop Java version 1.0)
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

public class Core
{
	static { System.loadLibrary("core"); }

	public static final int STATUS_ERROR_INVALIDPARAMS = -5;
	public static final int STATUS_ERROR_MEMORY = -4;
	public static final int STATUS_ERROR_AUDIOHW = -3;
	public static final int STATUS_ERROR_NOFILE = -2;
	public static final int STATUS_ERROR_GENERIC = -1;
	public static final int STATUS_UNINITIALIZED = 0;
	public static final int STATUS_READY = 1;
	public static final int STATUS_RUNNING = 2;
	public static final int STATUS_PAUSED = 3;
	public static final int STATUS_STOPPED = 4;

	public static native boolean initialize();
	public static native void deinitialize();
	public static native String getLastErrorMessage();
	public static native boolean setFileInDirectory(String fileDirectory);
	public static native boolean loadFile_createAudioObject();
	public static native boolean loadAudioDeviceList();
	public static native int getAudioDeviceListEntryCount();
	public static native String getAudioDeviceListEntry_friendlyName(int index);
	public static native boolean chooseDevice(int index, boolean enableResampling);
	public static native boolean chooseDefaultDevice(boolean enableResampling);
	public static native boolean initializeAudioObject();
	public static native int getStatus();
	public static native boolean runPlayback();
	public static native void pausePlayback();
	public static native void resumePlayback();
	public static native void stopPlayback();
	public static native long getAudioDataSizeFrames();
	public static native long getAudioDataPositionFrames();
	public static native boolean setAudioDataPositionFrames(long position);

	public static String formatSystemText(String text, String newLineReplacement, boolean replaceNewLine)
	{
		char[] input = null;
		String output = "";
		String newlinestr = "";
		int nChar;

		input = text.toCharArray();

		if(replaceNewLine) newlinestr = newLineReplacement;
		else newlinestr = System.lineSeparator();

		nChar = 0;
		while(nChar < input.length)
		{
			switch(input[nChar])
			{
				case '\r':
					if((nChar + 1) < input.length)
						if(input[nChar + 1] == '\n')
							nChar++;

				case '\n':
					output += newlinestr;
					break;

				default:
					output += input[nChar];
					break;
			}

			nChar++;
		}

		return output;
	}

	public static String formatSystemText(String text)
	{
		return Core.formatSystemText(text, "", false);
	}
}

