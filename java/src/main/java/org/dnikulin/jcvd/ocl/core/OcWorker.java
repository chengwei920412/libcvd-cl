package org.dnikulin.jcvd.ocl.core;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

import org.dnikulin.jcvd.core.Worker;

import com.nativelibs4java.opencl.CLContext;
import com.nativelibs4java.opencl.CLDevice;
import com.nativelibs4java.opencl.CLPlatform.ContextProperties;
import com.nativelibs4java.opencl.CLQueue;
import com.nativelibs4java.opencl.JavaCL;

public class OcWorker implements Worker {
	public final CLDevice  device;
	public final CLContext context;
	public final CLQueue   queue;

	public OcWorker(CLDevice device, CLContext context, CLQueue queue) {
		this.device  = device;
		this.context = context;
		this.queue   = queue;
	}

	public OcWorker(CLDevice device) {
		Map<ContextProperties, Object> props = new HashMap<ContextProperties, Object>();

		this.device  = device;
		this.context = JavaCL.createContext(props, this.device);
		this.queue   = device.createQueue(context);
	}

	@Override
	public String getName() {
		return device.getName();
	}

	@Override
	public void close() throws IOException {
		queue.release();
		context.release();
	}

	public CLDevice[] devices() {
		return new CLDevice[]{device};
	}
}
