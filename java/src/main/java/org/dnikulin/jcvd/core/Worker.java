package org.dnikulin.jcvd.core;

import java.io.Closeable;

public interface Worker extends Closeable {
	public String getName();
}
