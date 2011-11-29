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
