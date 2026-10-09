/*
 * WAVE Audio Playback for GNU-Linux systems.
 * Version 4.0 (Interop Java version 1.0)
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

import java.awt.event.ComponentEvent;
import java.awt.event.ComponentAdapter;
import java.awt.*;
import javax.swing.*;

public abstract class MyScreen extends JPanel
{
	protected JFrame parentWindow = null;

	protected ComponentAdapter componentAdapter = new ComponentAdapter() {
		@Override
		public void componentResized(ComponentEvent event)
		{
			align();
		}
	};

	public void init()
	{
		this.setLayout(new MyLayoutManager());
		this.setBackground(Definitions.MYSCREEN_BACKGROUNDCOLOR);

		this.parentWindow.addComponentListener(this.componentAdapter);
	}

	public void deinit()
	{
		this.parentWindow.removeComponentListener(this.componentAdapter);
	}

	protected abstract void align();
}

