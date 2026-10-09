/*
 * WAVE Audio Playback for GNU-Linux systems.
 * Version 4.0 (Interop Java version 1.0)
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

import java.awt.*;

/*
 * Empty implementation of LayoutManager.
 *
 * MyLayoutManager object is almost not used at all in the code.
 *
 * Its only purpose is to "exist", (to be a valid instance of a LayoutManager object),
 * so that "setLocation()" methods work properly
 */

public class MyLayoutManager implements LayoutManager
{
	@Override
	public void addLayoutComponent(String name, Component comp) {}

	@Override
	public void removeLayoutComponent(Component comp) {}

	@Override
	public Dimension preferredLayoutSize(Container parent) { return null; }

	@Override
	public Dimension minimumLayoutSize(Container parent) { return null; }

	@Override
	public void layoutContainer(Container parent) {}
}

