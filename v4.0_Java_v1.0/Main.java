/*
 * WAVE Audio Playback for GNU-Linux systems.
 * Version 4.0 (Interop Java version 1.0)
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

import java.io.File;
import java.awt.event.WindowEvent;
import java.awt.event.WindowAdapter;
import javax.swing.JFrame;
import javax.swing.JOptionPane;

public class Main
{
	private static class AudioThread extends Thread {
		@Override
		public void run() { Main.procAudioThread(); }
	};

	public static JFrame mainWnd = null;
	public static MyScreen screen = null;
	public static AudioThread audioThread = null;
	public static File fileObj = null;

	public static WindowAdapter windowAdapter = new WindowAdapter() {
		@Override
		public void windowClosed(WindowEvent event)
		{
			Main.appDeinit();
		}
	};

	public static void main(String[] args)
	{
		if(!Main.appInit()) return;

		Main.screen = new ChooseFileScreen(Main.mainWnd);
		Main.mainWnd.add(Main.screen);
		Main.mainWnd.setVisible(true);
	}

	public static boolean appInit()
	{
		if(!Core.initialize())
		{
			JOptionPane.showMessageDialog(new JFrame(), Core.getLastErrorMessage(), "INIT ERROR", JOptionPane.ERROR_MESSAGE);
			System.out.println(Core.getLastErrorMessage());
			return false;
		}

		Main.mainWnd = new JFrame();
		Main.mainWnd.setTitle(Definitions.MAINWND_CAPTION);
		Main.mainWnd.setSize(Definitions.MAINWND_INITSIZE);
		Main.mainWnd.addWindowListener(Main.windowAdapter);
		Main.mainWnd.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

		return true;
	}

	public static void appDeinit()
	{
		Core.deinitialize();
	}

	public static void appExit(int exitCode, String errorMessage)
	{
		Main.appDeinit();

		JOptionPane.showMessageDialog(new JFrame(), errorMessage, "PROCESS EXIT CALLED", JOptionPane.ERROR_MESSAGE);
		System.exit(exitCode);
	}

	public static void procLoadFileCreateAudioObject()
	{
		if(!Core.loadFile_createAudioObject())
		{
			JOptionPane.showMessageDialog(new JFrame(), Core.getLastErrorMessage(), "ERROR", JOptionPane.ERROR_MESSAGE);
			return;
		}

		Main.switchToChooseAudioDeviceScreen();
	}

	public static void procInitAudioObject()
	{
		if(!Core.initializeAudioObject())
		{
			JOptionPane.showMessageDialog(new JFrame(), Core.getLastErrorMessage(), "ERROR", JOptionPane.ERROR_MESSAGE);
			return;
		}

		Main.audioThread = new Main.AudioThread();
		Main.audioThread.start();

		Main.switchToPlaybackRunningScreen();
	}

	public static void switchToChooseFileScreen()
	{
		Main.screen.deinit();
		Main.mainWnd.remove(Main.screen);
		Main.screen = new ChooseFileScreen(Main.mainWnd);
		Main.mainWnd.add(Main.screen);
		Main.mainWnd.revalidate();
	}

	public static void switchToChooseAudioDeviceScreen()
	{
		Main.screen.deinit();
		Main.mainWnd.remove(Main.screen);
		Main.screen = new ChooseAudioDeviceScreen(Main.mainWnd);
		Main.mainWnd.add(Main.screen);
		Main.mainWnd.revalidate();
	}

	public static void switchToPlaybackRunningScreen()
	{
		Main.screen.deinit();
		Main.mainWnd.remove(Main.screen);
		Main.screen = new PlaybackRunningScreen(Main.mainWnd);
		Main.mainWnd.add(Main.screen);
		Main.mainWnd.revalidate();
	}

	public static void switchToPlaybackFinishedScreen()
	{
		Main.screen.deinit();
		Main.mainWnd.remove(Main.screen);
		Main.screen = new PlaybackFinishedScreen(Main.mainWnd);
		Main.mainWnd.add(Main.screen);
		Main.mainWnd.revalidate();
	}

	public static void procAudioThread()
	{
		Core.runPlayback();
		Main.switchToPlaybackFinishedScreen();
	}
}

