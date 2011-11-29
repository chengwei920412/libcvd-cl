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
import org.dnikulin.jcvd.ocl.core.OcWorker;

import com.nativelibs4java.opencl.CLBuffer;
import com.nativelibs4java.opencl.CLMem.Usage;

public class OcListState<T> extends OcCountState {
	public final Class<T> itemClass;

	public final int  each;
	public final long total;
	public final long bytes;

	public final CLBuffer<T> itemBuffer;
	public final Pointer<T>  itemStage;

	public OcListState(OcWorker worker, Class<T> itemClass, int size, int each) {
		super(worker, size);
		assert(each > 0);

		this.itemClass   = itemClass;
		this.each        = each;
		this.total       = ((long) size * (long) each);

		this.itemBuffer  = worker.context.createBuffer(Usage.InputOutput, itemClass, total);
		this.itemStage   = Pointer.allocate(itemClass);

		this.bytes       = itemBuffer.getByteCount();
	}

	@Override
	public void close() throws IOException {
		itemStage.release();
		itemBuffer.release();
		super.close();
	}

	public void write() {
		itemBuffer.write(worker.queue, itemStage, true);
	}

	public void read() {
		itemBuffer.read(worker.queue, itemStage, true);
	}

	public void zero() {
		itemStage.clearValidBytes();
	}

	public void writeZero() {
		zero();
		write();
	}
}
