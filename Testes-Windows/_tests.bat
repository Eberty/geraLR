rm *.out *.tbl *.dot *.svg *.lda *.tok *.mtk *.err *.html

.\geraLR.exe -w 0 -yndfoxlsLSNDA -C bege_marrom.cfg gram00.grm
.\geraLR.exe -w 0 -yndfoxlsLSNDA -C bege_marrom.cfg gram01.grm
.\geraLR.exe -w 0 -yndfoxlsLSNDA -C bege_marrom.cfg gram02.grm
.\geraLR.exe -w 0 -yndfoxlsLSNDA -C bege_marrom.cfg gram03.grm
.\geraLR.exe -w 0 -yndfoxlsLSNDA -C bege_marrom.cfg gram04a.grm
.\geraLR.exe -w 0 -yndfoxlsLSNDA -C bege_marrom.cfg gram04b.grm
.\geraLR.exe -w 0 -yndfoxlsLSNDA -C bege_marrom.cfg gram05.grm
.\geraLR.exe -w 0 -yndfoxlsLSNDA -C bege_marrom.cfg gram06.grm
.\geraLR.exe -w 0 -yndfoxlsLSNDA -C bege_marrom.cfg gram07.grm
.\geraLR.exe -w 0 -yndfoxlsLSNDA -C bege_marrom.cfg gram08.grm
.\geraLR.exe -w 0 -yndfoxlsLSNDA -C bege_marrom.cfg gram09a.grm
.\geraLR.exe -w 0 -yndfoxlsLSNDA -C bege_marrom.cfg gram09b.grm
.\geraLR.exe -w 0 -yndfoxlsLSNDA -C bege_marrom.cfg gram10.grm
.\geraLR.exe -w 0 -yndfoxlsLSNDA -C bege_marrom.cfg gram11.grm
.\geraLR.exe -w 0 -yndfoxlsLSNDA -C bege_marrom.cfg gram12.grm

.\lidas.exe -d LiDAS.mel -p gram00.grm-NFA.lda gram00.grm-NFA.svg
.\lidas.exe -d LiDAS.mel -p gram01.grm-NFA.lda gram01.grm-NFA.svg
.\lidas.exe -d LiDAS.mel -p gram02.grm-NFA.lda gram02.grm-NFA.svg
.\lidas.exe -d LiDAS.mel -p gram03.grm-NFA.lda gram03.grm-NFA.svg
.\lidas.exe -d LiDAS.mel -p gram04a.grm-NFA.lda gram04a.grm-NFA.svg
.\lidas.exe -d LiDAS.mel -p gram04b.grm-NFA.lda gram04b.grm-NFA.svg
.\lidas.exe -d LiDAS.mel -p gram05.grm-NFA.lda gram05.grm-NFA.svg
.\lidas.exe -d LiDAS.mel -p gram06.grm-NFA.lda gram06.grm-NFA.svg
.\lidas.exe -d LiDAS.mel -p gram07.grm-NFA.lda gram07.grm-NFA.svg
.\lidas.exe -d LiDAS.mel -p gram08.grm-NFA.lda gram08.grm-NFA.svg
.\lidas.exe -d LiDAS.mel -p gram09a.grm-NFA.lda gram09a.grm-NFA.svg
.\lidas.exe -d LiDAS.mel -p gram09b.grm-NFA.lda gram09b.grm-NFA.svg
.\lidas.exe -d LiDAS.mel -p gram10.grm-NFA.lda gram10.grm-NFA.svg
.\lidas.exe -d LiDAS.mel -p gram11.grm-NFA.lda gram11.grm-NFA.svg
.\lidas.exe -d LiDAS.mel -p gram12.grm-NFA.lda gram12.grm-NFA.svg

.\lidas.exe -d LiDAS.mel -p gram00.grm-DFA.lda gram00.grm-DFA.svg
.\lidas.exe -d LiDAS.mel -p gram01.grm-DFA.lda gram01.grm-DFA.svg
.\lidas.exe -d LiDAS.mel -p gram02.grm-DFA.lda gram02.grm-DFA.svg
.\lidas.exe -d LiDAS.mel -p gram03.grm-DFA.lda gram03.grm-DFA.svg
.\lidas.exe -d LiDAS.mel -p gram04a.grm-DFA.lda gram04a.grm-DFA.svg
.\lidas.exe -d LiDAS.mel -p gram04b.grm-DFA.lda gram04b.grm-DFA.svg
.\lidas.exe -d LiDAS.mel -p gram05.grm-DFA.lda gram05.grm-DFA.svg
.\lidas.exe -d LiDAS.mel -p gram06.grm-DFA.lda gram06.grm-DFA.svg
.\lidas.exe -d LiDAS.mel -p gram07.grm-DFA.lda gram07.grm-DFA.svg
.\lidas.exe -d LiDAS.mel -p gram08.grm-DFA.lda gram08.grm-DFA.svg
.\lidas.exe -d LiDAS.mel -p gram09a.grm-DFA.lda gram09a.grm-DFA.svg
.\lidas.exe -d LiDAS.mel -p gram09b.grm-DFA.lda gram09b.grm-DFA.svg
.\lidas.exe -d LiDAS.mel -p gram10.grm-DFA.lda gram10.grm-DFA.svg
.\lidas.exe -d LiDAS.mel -p gram11.grm-DFA.lda gram11.grm-DFA.svg
.\lidas.exe -d LiDAS.mel -p gram12.grm-DFA.lda gram12.grm-DFA.svg
