package org.dnikulin.jcvd.ocl.step;

import java.io.IOException;

import org.dnikulin.jcvd.ocl.core.OcWorker;
import org.dnikulin.jcvd.ocl.help.OcSimpleStep;

public class OcPreFastStep extends OcSimpleStep {
	public OcPreFastStep(OcWorker worker) throws IOException {
		super(worker);
		buildKernel();
	}

	@Override
	public void run() {
		
	}

	@Override
	public String getResourceName() {
		return "pre-fast-gray.cl";
	}

	@Override
	public String getKernelName() {
		return "pre_fast_gray";
	}
}
