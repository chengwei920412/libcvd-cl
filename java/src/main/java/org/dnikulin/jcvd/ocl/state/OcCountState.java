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

package org.dnikulin.jcvd.ocl.state;

import java.io.IOException;

import org.bridj.Pointer;
import org.dnikulin.jcvd.ocl.core.OcWorkState;
import org.dnikulin.jcvd.ocl.core.OcWorker;

import com.nativelibs4java.opencl.CLBuffer;
import com.nativelibs4java.opencl.CLMem.Usage;

public class OcCountState extends OcWorkState {
	public final int size;

	public final CLBuffer<Integer> countBuffer;
	public final Pointer<Integer>  countStage;

	public OcCountState(OcWorker worker, int size) {
		super(worker);
		assert(size > 0);

		this.size        = size;

		this.countBuffer = worker.context.createIntBuffer(Usage.InputOutput, 1);
		this.countStage  = Pointer.allocateInt();
	}

	@Override
	public void close() throws IOException {
		countStage.release();
		countBuffer.release();
	}

	public void setCount(int value) {
		countStage.setInt(value);
		countBuffer.write(worker.queue, countStage, true);
	}

	public int getCount() {
		countBuffer.read(worker.queue, countStage, true);
		return countStage.getInt();
	}
}
