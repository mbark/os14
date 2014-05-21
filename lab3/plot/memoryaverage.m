hold on;

nf_x = [1 10 100 1000 10000];
nf_y_mat = [45 21 9 8 7 ; 60 11 10 8 7 ; 98 28 12 9 7 ; 2 20 16 9 7 ; 4 15 6 9 7];
nf_y = mean(nf_y_mat);
nf_std = std(nf_y_mat);

errorbar(nf_x, nf_y, nf_std, 'b');

ss_x = [1 10 100 1000 10000];
ss_y_mat = [91 43 12 0 0 ; 97 58 0 0 0 ; 90 44 13 1 0 ; 89 38 8 1 0 ; 91 48 13 1 0];
ss_y = mean(ss_y_mat);
ss_std = std(ss_y_mat);

errorbar(ss_x, ss_y, ss_std, 'g');

wf_x = [1 10 100 1000 10000];
wf_y_mat = [62 16 15 15 14 ; 40 27 14 15 14 ; 41 14 16 15 14 ; 25 17 16 15 14 ; 68 12 16 14 14];
wf_y = mean(wf_y_mat);
wf_std = std(wf_y_mat);

h = errorbar(wf_x, wf_y, wf_std, 'r');
set(get(h,'Parent'), 'XScale', 'log');