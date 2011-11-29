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
