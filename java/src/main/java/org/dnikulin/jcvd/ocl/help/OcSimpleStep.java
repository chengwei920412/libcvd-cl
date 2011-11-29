// Copyright (C) 2011  Dmitri Nikulin
// Copyright (C) 2011  Monash University
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use,
// copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following
// conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.

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
