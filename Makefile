all: parse
	
cme:
	rcp kal@web626.webfaction.com:cme/stleqt/stleqt.20171109.gz .

parse:
	./stleqt.pl stleqt.20171109
