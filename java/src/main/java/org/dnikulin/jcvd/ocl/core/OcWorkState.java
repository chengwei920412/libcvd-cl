package org.dnikulin.jcvd.ocl.core;

import org.dnikulin.jcvd.core.WorkState;

public abstract class OcWorkState extends HasOcWorker implements WorkState {
	public OcWorkState(OcWorker worker) {
		super(worker);
	}
}
