package org.dnikulin.jcvd.core;

import java.io.Closeable;

public interface WorkStep extends HasWorker, Closeable, Runnable {
}
