(set-info :status sat)
(set-option :produce-models true)
(declare-const x (Array Bool Bool))
(declare-const _x (Array Bool Bool))
(assert (= x (store (store _x true false) true (exists ((x Bool)) true))))
(check-sat)