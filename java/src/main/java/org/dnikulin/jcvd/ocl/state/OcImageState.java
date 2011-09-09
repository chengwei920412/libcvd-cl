package org.dnikulin.jcvd.ocl.state;

import java.awt.image.BufferedImage;
import java.io.IOException;

import org.dnikulin.jcvd.ocl.core.OcWorkState;
import org.dnikulin.jcvd.ocl.core.OcWorker;

import com.nativelibs4java.opencl.CLImage2D;
import com.nativelibs4java.opencl.CLImageFormat;
import com.nativelibs4java.opencl.CLMem.Usage;

public class OcImageState extends OcWorkState {
	public final int nx;
	public final int ny;
	public final int nxy;

	public final CLImageFormat format;
	public final CLImage2D     imageBuffer;

	public OcImageState(OcWorker worker, CLImageFormat format, int nx, int ny) {
		super(worker);
		assert(nx > 0);
		assert(ny > 0);

		this.nx          = nx;
		this.ny          = ny;
		this.nxy         = (nx * ny);

		this.format      = format;
		this.imageBuffer = worker.context.createImage2D(Usage.InputOutput, format, nx, ny);
	}

	@Override
	public void close() throws IOException {
		imageBuffer.release();
	}

	public void write(BufferedImage image) {
		imageBuffer.write(worker.queue, image, true);
	}

	public void writeGray(byte[] bytes) {
		write(new BufferedImage(nx, ny, BufferedImage.TYPE_BYTE_GRAY));
	}
}
