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
