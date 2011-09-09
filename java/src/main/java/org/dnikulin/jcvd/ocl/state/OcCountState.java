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
