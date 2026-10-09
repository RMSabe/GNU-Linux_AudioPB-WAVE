/*
 * WAVE Audio Playback for GNU-Linux systems.
 * Version 4.0 (Interop Java version 1.0)
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.*;
import javax.swing.*;

public class ChooseFileScreen extends MyScreen
{
	private static final String ACTIONCOMMAND_CHOOSEFILE = "CHOOSEFILE";

	private JLabel jlabel1 = new JLabel();
	private JButton jbutton1 = new JButton();

	private ActionListener actionListener = new ActionListener() {
		@Override
		public void actionPerformed(ActionEvent event)
		{
			if(event.getActionCommand().equals(ChooseFileScreen.ACTIONCOMMAND_CHOOSEFILE)) procOpenFileDialog();
		}
	};

	public ChooseFileScreen(JFrame parentWindow)
	{
		this.parentWindow = parentWindow;
		this.init();
	}

	@Override
	public void init()
	{
		super.init();

		this.jlabel1.setFont(Definitions.TITLE_FONT);
		this.jlabel1.setForeground(Definitions.TEXT_FOREGROUNDCOLOR);
		this.jlabel1.setHorizontalAlignment(JLabel.CENTER);
		this.jlabel1.setText("Select Input File");
		this.jlabel1.setVisible(true);

		this.jbutton1.setBackground(Definitions.BUTTON_BACKGROUNDCOLOR);
		this.jbutton1.setForeground(Definitions.BUTTON_FOREGROUNDCOLOR);
		this.jbutton1.addActionListener(this.actionListener);
		this.jbutton1.setActionCommand(ChooseFileScreen.ACTIONCOMMAND_CHOOSEFILE);
		this.jbutton1.setFocusable(true);
		this.jbutton1.setText("Search File");
		this.jbutton1.setVisible(true);

		this.add(this.jlabel1);
		this.add(this.jbutton1);

		this.align();
	}

	@Override
	public void deinit()
	{
		super.deinit();

		this.jbutton1.removeActionListener(this.actionListener);
		this.remove(this.jlabel1);
		this.remove(this.jbutton1);
	}

	@Override
	public void align()
	{
		Dimension parentSize = this.parentWindow.getSize();

		Point center = new Point();

		Point jlabel1_pos = new Point();
		Dimension jlabel1_size = new Dimension();

		Point jbutton1_pos = new Point();
		Dimension jbutton1_size = new Dimension();

		this.setSize(parentSize);

		center.x = parentSize.width/2;
		center.y = parentSize.height/2;

		jlabel1_pos.x = Definitions.TITLE_MARGINLEFT;
		jlabel1_pos.y = Definitions.TITLE_MARGINTOP;

		jlabel1_size.width = parentSize.width - 2*jlabel1_pos.x;
		jlabel1_size.height = Definitions.TITLE_FONTSIZE + Definitions.TITLE_FONTSIZEMARGIN;

		jbutton1_size.width = 120;
		jbutton1_size.height = 20;

		jbutton1_pos.x = center.x - jbutton1_size.width/2;
		jbutton1_pos.y = parentSize.height - jbutton1_size.height - Definitions.BOTTOM_WINDOW_MARGIN;

		this.jlabel1.setSize(jlabel1_size);
		this.jlabel1.setLocation(jlabel1_pos);

		this.jbutton1.setSize(jbutton1_size);
		this.jbutton1.setLocation(jbutton1_pos);
	}

	private void procOpenFileDialog()
	{
		JFileChooser jfilechooser = null;
		int nRet = 0;

		if(Main.fileObj != null) Main.fileObj = Main.fileObj.getParentFile();

		if(Main.fileObj != null) jfilechooser = new JFileChooser(Main.fileObj);
		else jfilechooser = new JFileChooser();

		nRet = jfilechooser.showOpenDialog(new JFrame());

		if(nRet == JFileChooser.APPROVE_OPTION)
		{
			Main.fileObj = jfilechooser.getSelectedFile();
			Core.setFileInDirectory(Main.fileObj.getAbsolutePath());
			Main.procLoadFileCreateAudioObject();
		}
	}
}

