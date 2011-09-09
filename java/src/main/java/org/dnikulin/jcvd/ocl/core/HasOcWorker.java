package org.dnikulin.jcvd.ocl.core;

import org.dnikulin.jcvd.core.HasWorker;

public class HasOcWorker implements HasWorker {
	public final OcWorker worker;

	public HasOcWorker(OcWorker worker) {
		this.worker = worker;
	}

	@Override
	public OcWorker getWorker() {
		return worker;
	}
}
