/*
 * WAVE Audio Playback for GNU-Linux systems.
 * Version 4.0 (Interop Java version 1.0)
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import java.awt.*;
import javax.swing.*;

public class PlaybackRunningScreen extends MyScreen
{
	private static final String ACTIONCOMMAND_STOPPLAYBACK = "STOPPLAYBACK";
	private static final String ACTIONCOMMAND_PAUSERESUMEPLAYBACK = "PAUSERESUMEPLAYBACK";

	private JLabel jlabel_title = new JLabel();
	private JButton jbutton_stoppb = new JButton();
	private JButton jbutton_pauseresumepb = new JButton();
	private JSlider jslider_timecursor = new JSlider();

	private boolean thread_timecursor_stop = false;
	private boolean thread_timecursor_bypass = false;

	private Thread thread_timecursor = new Thread() {
		@Override
		public void run()
		{
			long audiodata_pos = 0L;

			while(!thread_timecursor_stop)
			{
				switch(Core.getStatus())
				{
					case Core.STATUS_RUNNING:
					case Core.STATUS_PAUSED:

						if(jslider_timecursor.getValueIsAdjusting() || thread_timecursor_bypass) break;

						audiodata_pos = Core.getAudioDataPositionFrames();
						if(audiodata_pos >= 0L)
						{
							jslider_timecursor.setEnabled(false);
							jslider_timecursor.setValue((int) audiodata_pos);
							jslider_timecursor.setEnabled(true);
						}

						break;

					default:
						thread_timecursor_stop = true;
						return;
				}

				try { Thread.sleep(1024); }
				catch(Exception _e) { return; }
			}
		}
	};

	private ActionListener actionListener = new ActionListener() {
		@Override
		public void actionPerformed(ActionEvent event)
		{
			String eventCmd = event.getActionCommand();

			if(eventCmd.equals(PlaybackRunningScreen.ACTIONCOMMAND_STOPPLAYBACK))
			{
				Core.stopPlayback();
				return;
			}

			if(eventCmd.equals(PlaybackRunningScreen.ACTIONCOMMAND_PAUSERESUMEPLAYBACK))
			{
				switch(Core.getStatus())
				{
					case Core.STATUS_RUNNING:
						Core.pausePlayback();
						jbutton_pauseresumepb.setText("Resume Playback");
						break;

					case Core.STATUS_PAUSED:
						Core.resumePlayback();
						jbutton_pauseresumepb.setText("Pause Playback");
						break;
				}

				return;
			}
		}
	};

	private ChangeListener changeListener = new ChangeListener() {
		@Override
		public void stateChanged(ChangeEvent event)
		{
			if(jslider_timecursor.getValueIsAdjusting()) thread_timecursor_bypass = true;
			else
			{
				Core.setAudioDataPositionFrames((long) jslider_timecursor.getValue());
				thread_timecursor_bypass = false;
			}
		}
	};

	public PlaybackRunningScreen(JFrame parentWindow)
	{
		this.parentWindow = parentWindow;
		this.init();
	}

	@Override
	public void init()
	{
		long audiodata_size = 0L;

		super.init();

		this.jlabel_title.setForeground(Definitions.TEXT_FOREGROUNDCOLOR);
		this.jlabel_title.setFont(Definitions.TITLE_FONT);
		this.jlabel_title.setHorizontalAlignment(JLabel.CENTER);
		this.jlabel_title.setText("Playback Running");

		this.jbutton_stoppb.setBackground(Definitions.BUTTON_BACKGROUNDCOLOR);
		this.jbutton_stoppb.setForeground(Definitions.BUTTON_FOREGROUNDCOLOR);
		this.jbutton_stoppb.addActionListener(this.actionListener);
		this.jbutton_stoppb.setActionCommand(PlaybackRunningScreen.ACTIONCOMMAND_STOPPLAYBACK);
		this.jbutton_stoppb.setText("Stop Playback");
		this.jbutton_stoppb.setFocusable(true);
		this.jbutton_stoppb.setVisible(true);

		this.jbutton_pauseresumepb.setBackground(Definitions.BUTTON_BACKGROUNDCOLOR);
		this.jbutton_pauseresumepb.setForeground(Definitions.BUTTON_FOREGROUNDCOLOR);
		this.jbutton_pauseresumepb.addActionListener(this.actionListener);
		this.jbutton_pauseresumepb.setActionCommand(PlaybackRunningScreen.ACTIONCOMMAND_PAUSERESUMEPLAYBACK);
		this.jbutton_pauseresumepb.setText("Pause Playback");
		this.jbutton_pauseresumepb.setFocusable(true);
		this.jbutton_pauseresumepb.setVisible(true);

		audiodata_size = Core.getAudioDataSizeFrames();
		if(audiodata_size > 0L)
		{
			this.jslider_timecursor.setMinimum(0);
			this.jslider_timecursor.setMaximum((int) audiodata_size);
			this.jslider_timecursor.setValue(0);
			this.jslider_timecursor.addChangeListener(this.changeListener);
			this.jslider_timecursor.setVisible(true);
		}
		else this.jslider_timecursor.setVisible(false);

		this.add(this.jlabel_title);
		this.add(this.jbutton_stoppb);
		this.add(this.jbutton_pauseresumepb);
		this.add(this.jslider_timecursor);

		this.align();
		this.thread_timecursor_stop = false;
		this.thread_timecursor_bypass = false;
		this.thread_timecursor.start();
	}

	@Override
	public void deinit()
	{
		this.thread_timecursor_stop = true;

		try { this.thread_timecursor.join(); }
		catch(Exception _e) {}

		super.deinit();

		this.jbutton_stoppb.removeActionListener(this.actionListener);
		this.jbutton_pauseresumepb.removeActionListener(this.actionListener);
		this.jslider_timecursor.removeChangeListener(this.changeListener);

		this.remove(this.jlabel_title);
		this.remove(this.jbutton_stoppb);
		this.remove(this.jbutton_pauseresumepb);
		this.remove(this.jslider_timecursor);
	}

	@Override
	public void align()
	{
		Dimension parentSize = this.parentWindow.getSize();

		Point center = new Point();

		Point title_pos = new Point();
		Dimension title_size = new Dimension();

		Point b_stoppb_pos = new Point();
		Dimension b_stoppb_size = new Dimension();

		Point b_pauseresumepb_pos = new Point();
		Dimension b_pauseresumepb_size = new Dimension();

		Point s_timecursor_pos = new Point();
		Dimension s_timecursor_size = new Dimension();

		this.setSize(parentSize);

		center.x = parentSize.width/2;
		center.y = parentSize.height/2;

		title_pos.x = Definitions.TITLE_MARGINLEFT;
		title_pos.y = Definitions.TITLE_MARGINTOP;

		title_size.width = parentSize.width - 2*title_pos.x;
		title_size.height = Definitions.TITLE_FONTSIZE + Definitions.TITLE_FONTSIZEMARGIN;

		b_pauseresumepb_size.width = 200;
		b_pauseresumepb_size.height = 20;

		b_pauseresumepb_pos.x = center.x - b_pauseresumepb_size.width - Definitions.INTERCOMPONENT_MARGIN/2;
		b_pauseresumepb_pos.y = parentSize.height - b_pauseresumepb_size.height - Definitions.BOTTOM_WINDOW_MARGIN;

		b_stoppb_size.width = 200;
		b_stoppb_size.height = 20;

		b_stoppb_pos.x = center.x + Definitions.INTERCOMPONENT_MARGIN/2;
		b_stoppb_pos.y = parentSize.height - b_stoppb_size.height - Definitions.BOTTOM_WINDOW_MARGIN;

		s_timecursor_size.width = b_pauseresumepb_size.width + b_stoppb_size.width + Definitions.INTERCOMPONENT_MARGIN;
		s_timecursor_size.height = 20;

		s_timecursor_pos.x = center.x - s_timecursor_size.width/2;
		s_timecursor_pos.y = b_pauseresumepb_pos.y - s_timecursor_size.height - Definitions.INTERCOMPONENT_MARGIN;

		this.jlabel_title.setSize(title_size);
		this.jlabel_title.setLocation(title_pos);

		this.jbutton_pauseresumepb.setSize(b_pauseresumepb_size);
		this.jbutton_pauseresumepb.setLocation(b_pauseresumepb_pos);

		this.jbutton_stoppb.setSize(b_stoppb_size);
		this.jbutton_stoppb.setLocation(b_stoppb_pos);

		this.jslider_timecursor.setSize(s_timecursor_size);
		this.jslider_timecursor.setLocation(s_timecursor_pos);
	}
}

