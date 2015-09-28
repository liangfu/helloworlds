lb.pdf: lb_v3.gv
	cat lb_v3.gv | neato -Tpdf -n2 -Knop > lb.pdf && evince lb.pdf

lb_v3.gv: l.g lb_v2.gv
	cat lb_v2.gv | gvpr -c -f l.g -o lb_v3.gv

lb_v2.gv: lb.gv
	dot lb.gv > lb_v2.gv

.PHONY: lb.pdf lb_v3.gv lb_v2.gv

