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

public class ChooseAudioDeviceScreen extends MyScreen
{
	private static final String ACTIONCOMMAND_RETURNCHOOSEFILE = "RETURNCHOOSEFILE";
	private static final String ACTIONCOMMAND_CHOOSESELECTEDDEV = "CHOOSESELECTEDDEV";
	private static final String ACTIONCOMMAND_CHOOSEDEFAULTDEV = "CHOOSEDEFAULTDEV";

	private static final int DEVICELIST_HEIGHT = 200;

	private JLabel jlabel1 = new JLabel();
	private JButton jbutton1 = new JButton();
	private JButton jbutton2 = new JButton();
	private JButton jbutton3 = new JButton();
	private JCheckBox jcheckbox1 = new JCheckBox();
	private JScrollPane jscrollpane1 = null;
	private JList<String> jlist1 = null;
	private String[] devList = null;

	private ActionListener actionListener = new ActionListener() {
		@Override
		public void actionPerformed(ActionEvent event)
		{
			int nSel;

			if(event.getActionCommand().equals(ChooseAudioDeviceScreen.ACTIONCOMMAND_RETURNCHOOSEFILE))
			{
				Main.switchToChooseFileScreen();
				return;
			}

			if(event.getActionCommand().equals(ChooseAudioDeviceScreen.ACTIONCOMMAND_CHOOSESELECTEDDEV))
			{
				nSel = jlist1.getSelectedIndex();
				if(nSel < 0)
				{
					JOptionPane.showMessageDialog(new JFrame(), "Error: no device selected.", "ERROR", JOptionPane.ERROR_MESSAGE);
					return;
				}

				if(!Core.chooseDevice(nSel, jcheckbox1.isSelected()))
				{
					JOptionPane.showMessageDialog(new JFrame(), Core.getLastErrorMessage(), "ERROR", JOptionPane.ERROR_MESSAGE);
					return;
				}

				Main.procInitAudioObject();
				return;
			}

			if(event.getActionCommand().equals(ChooseAudioDeviceScreen.ACTIONCOMMAND_CHOOSEDEFAULTDEV))
			{
				if(!Core.chooseDefaultDevice(jcheckbox1.isSelected()))
				{
					JOptionPane.showMessageDialog(new JFrame(), Core.getLastErrorMessage(), "ERROR", JOptionPane.ERROR_MESSAGE);
					return;
				}

				Main.procInitAudioObject();
				return;
			}
		}
	};

	public ChooseAudioDeviceScreen(JFrame parentWindow)
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
		this.jlabel1.setText("Choose Audio Device:");
		this.jlabel1.setVisible(true);

		this.jbutton1.setBackground(Definitions.BUTTON_BACKGROUNDCOLOR);
		this.jbutton1.setForeground(Definitions.BUTTON_FOREGROUNDCOLOR);
		this.jbutton1.addActionListener(this.actionListener);
		this.jbutton1.setActionCommand(ChooseAudioDeviceScreen.ACTIONCOMMAND_RETURNCHOOSEFILE);
		this.jbutton1.setFocusable(true);
		this.jbutton1.setText("Return");
		this.jbutton1.setVisible(true);

		this.jbutton2.setBackground(Definitions.BUTTON_BACKGROUNDCOLOR);
		this.jbutton2.setForeground(Definitions.BUTTON_FOREGROUNDCOLOR);
		this.jbutton2.addActionListener(this.actionListener);
		this.jbutton2.setActionCommand(ChooseAudioDeviceScreen.ACTIONCOMMAND_CHOOSESELECTEDDEV);
		this.jbutton2.setFocusable(true);
		this.jbutton2.setText("Choose Selected Device");

		this.jbutton3.setBackground(Definitions.BUTTON_BACKGROUNDCOLOR);
		this.jbutton3.setForeground(Definitions.BUTTON_FOREGROUNDCOLOR);
		this.jbutton3.addActionListener(this.actionListener);
		this.jbutton3.setActionCommand(ChooseAudioDeviceScreen.ACTIONCOMMAND_CHOOSEDEFAULTDEV);
		this.jbutton3.setFocusable(true);
		this.jbutton3.setText("Choose Default Device");
		this.jbutton3.setVisible(true);

		this.jcheckbox1.setBackground(Color.WHITE);
		this.jcheckbox1.setForeground(Color.BLACK);
		this.jcheckbox1.setFocusable(true);
		this.jcheckbox1.setText("Enable Device Resampling. (Enabled = Better Compatibility / Disabled = Better Performance). (Recommended: Enabled)");
		this.jcheckbox1.setVisible(true);

		this.add(this.jlabel1);
		this.add(this.jbutton1);
		this.add(this.jbutton2);
		this.add(this.jbutton3);
		this.add(this.jcheckbox1);
		this.loadDevList();

		this.align();
	}

	@Override
	public void deinit()
	{
		super.deinit();

		this.jbutton1.removeActionListener(this.actionListener);
		this.jbutton2.removeActionListener(this.actionListener);
		this.jbutton3.removeActionListener(this.actionListener);

		this.remove(this.jlabel1);
		this.remove(this.jbutton1);
		this.remove(this.jbutton2);
		this.remove(this.jbutton3);
		this.remove(this.jcheckbox1);

		if(this.jscrollpane1 != null)
		{
			if(this.jlist1 != null) this.jscrollpane1.remove(this.jlist1);

			this.remove(this.jscrollpane1);
		}
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

		Point jbutton2_pos = new Point();
		Dimension jbutton2_size = new Dimension();

		Point jbutton3_pos = new Point();
		Dimension jbutton3_size = new Dimension();

		Point jcheckbox1_pos = new Point();
		Dimension jcheckbox1_size = new Dimension();

		Point jscrollpane1_pos = new Point();
		Dimension jscrollpane1_size = new Dimension();

		this.setSize(parentSize);

		center.x = parentSize.width/2;
		center.y = parentSize.height/2;

		jlabel1_pos.x = Definitions.TITLE_MARGINLEFT;
		jlabel1_pos.y = Definitions.TITLE_MARGINTOP;

		jlabel1_size.width = parentSize.width - 2*jlabel1_pos.x;
		jlabel1_size.height = Definitions.TITLE_FONTSIZE + Definitions.TITLE_FONTSIZEMARGIN;

		jscrollpane1_pos.x = jlabel1_pos.x;
		jscrollpane1_pos.y = jlabel1_pos.y + jlabel1_size.height + Definitions.INTERCOMPONENT_MARGIN;

		jscrollpane1_size.width = parentSize.width - 2*jscrollpane1_pos.x;
		jscrollpane1_size.height = ChooseAudioDeviceScreen.DEVICELIST_HEIGHT;

		jcheckbox1_pos.x = jscrollpane1_pos.x;
		jcheckbox1_pos.y = jscrollpane1_pos.y + jscrollpane1_size.height + Definitions.INTERCOMPONENT_MARGIN;

		jcheckbox1_size.width = parentSize.width - 2*jcheckbox1_pos.x;
		jcheckbox1_size.height = 20;

		jbutton1_size.width = 260;
		jbutton1_size.height = 20;

		jbutton2_size.width = 260;
		jbutton2_size.height = 20;

		jbutton3_size.width = 260;
		jbutton3_size.height = 20;

		jbutton2_pos.x = center.x - jbutton2_size.width/2;
		jbutton2_pos.y = parentSize.height - jbutton2_size.height - Definitions.BOTTOM_WINDOW_MARGIN;

		jbutton1_pos.x = jbutton2_pos.x - jbutton1_size.width - Definitions.INTERCOMPONENT_MARGIN;
		jbutton1_pos.y = parentSize.height - jbutton1_size.height - Definitions.BOTTOM_WINDOW_MARGIN;

		jbutton3_pos.x = jbutton2_pos.x + jbutton2_size.width + Definitions.INTERCOMPONENT_MARGIN;
		jbutton3_pos.y = parentSize.height - jbutton3_size.height - Definitions.BOTTOM_WINDOW_MARGIN;

		this.jlabel1.setSize(jlabel1_size);
		this.jlabel1.setLocation(jlabel1_pos);

		this.jbutton1.setSize(jbutton1_size);
		this.jbutton1.setLocation(jbutton1_pos);

		this.jbutton2.setSize(jbutton2_size);
		this.jbutton2.setLocation(jbutton2_pos);

		this.jbutton3.setSize(jbutton3_size);
		this.jbutton3.setLocation(jbutton3_pos);

		this.jcheckbox1.setSize(jcheckbox1_size);
		this.jcheckbox1.setLocation(jcheckbox1_pos);

		if(this.jscrollpane1 != null)
		{
			this.jscrollpane1.setSize(jscrollpane1_size);
			this.jscrollpane1.setLocation(jscrollpane1_pos);
		}

		if(this.jlist1 != null) this.jlist1.setSize(jscrollpane1_size);
	}

	private void loadDevList()
	{
		int nDevices = 0;
		int nIndex = 0;

		this.devList = null;
		this.jscrollpane1 = null;
		this.jlist1 = null;

		if(!Core.loadAudioDeviceList())
		{
			this.jbutton2.setVisible(false);
			JOptionPane.showMessageDialog(new JFrame(), Core.getLastErrorMessage(), "ERROR", JOptionPane.ERROR_MESSAGE);
			return;
		}

		nDevices = Core.getAudioDeviceListEntryCount();
		if(nDevices < 0)
		{
			this.jbutton2.setVisible(false);
			JOptionPane.showMessageDialog(new JFrame(), Core.getLastErrorMessage(), "ERROR", JOptionPane.ERROR_MESSAGE);
			return;
		}

		this.devList = new String[nDevices];

		nIndex = 0;
		while(nIndex < nDevices)
		{
			this.devList[nIndex] = Core.formatSystemText(Core.getAudioDeviceListEntry_friendlyName(nIndex), " : ", true);

			nIndex++;
		}

		this.jlist1 = new JList<String>(this.devList);
		this.jlist1.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
		this.jlist1.setVisible(true);

		this.jscrollpane1 = new JScrollPane(this.jlist1);
		this.jscrollpane1.setVisible(true);

		this.add(this.jscrollpane1);
		this.jbutton2.setVisible(true);
	}
}

