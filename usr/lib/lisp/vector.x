(File vector.l)
(set-in-fclosure lambda return + vset setf |1+| cdr null if vref eq lessp < vsize do error fclosurep not cond)
(symeval-in-fclosure lambda + |1+| cdr null if return vref eq lessp < vsize do error fclosurep not cond)
(vector-dump lambda |1+| vref lessp < not do terpr vprop vsize setq patom progn msg let)
(fclosure-function lambda vref fclosurep and)
(fclosure-alist lambda error + |1+| cdr null if vref cons setq setf push lessp < not vsize do fclosurep cond)
(fclosurep lambda vprop quote eq vectorp and)
(fclosure-list lexpr |1+| make-fclosure-with-alist setq setf push nreverse greaterp > list cons |1-| + cdr <& arg do minusp eq let listify error evenp not cond)
(fclosure lambda list make-fclosure-with-alist)
(make-fclosure-with-alist lambda list nconc boundp caddr cadr |1+| return caar eq assq symbolp car setq vset setf do quote length cdr + new-vector let error dtpr null or not cond)
(vsize-byte lambda error int:vsize vectorip cond if)
(vsize-word lambda error int:vsize vectorip cond if)
(vsize lambda error int:vsize vectorip vectorp or cond if)
(vseti-long lambda int:vset vsize cdr <& or fixp error vectorip not cond vset-macro)
(vseti-word lambda int:vset vsize-word cdr <& or fixp error vectorip not cond vset-macro)
(vseti-byte lambda int:vset vsize-byte cdr <& or fixp error vectorip not cond vset-macro)
(vset lambda int:vset vsize cdr <& or fixp error vectorp not cond vset-macro)
(vrefi-long lambda int:vref vsize lessp <& < or fixp error vectorip not cond vref-macro)
(vrefi-word lambda int:vref vsize cdr * lessp <& < or fixp error vectorip not cond vref-macro)
(vrefi-byte lambda int:vref vsize cdr * lessp <& < or fixp error vectorip not cond vref-macro)
(vref lambda int:vref vsize lessp <& < or fixp error vectorp not cond vref-macro)
(vectori-long lexpr cdr arg int:vset <& < |1-| do new-vectori-long let vector-macro)
(vectori-word lexpr cdr arg int:vset <& < |1-| do new-vectori-word let vector-macro)
(vectori-byte lexpr cdr arg int:vset <& < |1-| do new-vectori-byte let vector-macro)
(vector lexpr cdr arg int:vset <& < |1-| do new-vector let vector-macro)
