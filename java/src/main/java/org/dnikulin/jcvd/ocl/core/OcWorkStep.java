package org.dnikulin.jcvd.ocl.core;

import org.dnikulin.jcvd.core.WorkStep;

public abstract class OcWorkStep extends HasOcWorker implements WorkStep {
	public OcWorkStep(OcWorker worker) {
		super(worker);
	}
}
