package org.dnikulin.jcvd.ocl.help;

import java.io.IOException;
import java.io.InputStream;
import java.net.URL;

import org.dnikulin.jcvd.core.HasResourceName;
import org.dnikulin.jcvd.ocl.core.HasKernelName;
import org.dnikulin.jcvd.ocl.core.OcWorkStep;
import org.dnikulin.jcvd.ocl.core.OcWorker;

import com.nativelibs4java.opencl.CLDevice;
import com.nativelibs4java.opencl.CLException;
import com.nativelibs4java.opencl.CLKernel;
import com.nativelibs4java.opencl.CLProgram;
import com.nativelibs4java.util.IOUtils;

public abstract class OcSimpleStep extends OcWorkStep implements HasResourceName, HasKernelName {
	protected final CLProgram program;
	protected       CLKernel  kernel;

	public OcSimpleStep(OcWorker worker) throws IOException, CLException {
		super(worker);

		String resourceName  = getResourceName();

		ClassLoader  loader  = getClass().getClassLoader();
		URL          url     = loader.getResource(resourceName);
		InputStream  stream  = url.openStream();
		String       source  = IOUtils.readTextClose(stream);
		
		CLDevice []  devices = worker.devices();

		this.program = worker.context.createProgram(devices, source);
	}

	protected void buildKernel() {
		String kernelName = getKernelName();
		program.build();
		kernel = program.createKernel(kernelName);
	}

	@Override
	public void close() throws IOException {
		if (kernel != null)
			kernel.release();
		program.release();
	}
}
