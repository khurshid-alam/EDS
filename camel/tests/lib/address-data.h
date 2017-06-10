/*
 * This library is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

/* BE WARY of editing this file with emacs.
 * Otherwise it might be smart and try to re-encode everything, which
 * you really do not want
*/

static struct _a {
	gint count;
	const gchar *addr;
	const gchar *utf8;	/* The utf8 in this table was generated by
				 * Camel itself.  As a result I'm making the
				 * assumption it was right when it was created.
				 * It also depends on the format of ::format (),
				 * which is likely to change, to handle other
				 * bugs!*/
} test_address[] = {
	{ 1, "\"=?ISO-8859-1?Q?David_Guti=E9rrez_Magallanes?=\" <david@iiia.csic.es>", "David Gutiérrez Magallanes <david@iiia.csic.es>" },
	{ 1, "\"=?iso-8859-1?Q?Jos=E9?= Antonio Milke G.\" <gerencia@ovoplus.com>", "José Antonio Milke G. <gerencia@ovoplus.com>" },
	{ 1, "\"=?iso-8859-2?Q?Hi-Fi_Internert_market_=3D_1.Virtu=E1ln=ED_Internetov=E9_H?= =?iso-8859-2?Q?i-Fi_Studio?=\" <hifimarket@atlas.cz>", "Hi-Fi Internert market = 1.Virtuální Internetové Hi-Fi Studio <hifimarket@atlas.cz>" },
	{ 3, "\"James M. Cape\" <jcape@jcinteractive.com>, =?iso-8859-1?Q?Joaqu=EDn_Cuenca_Abela?= <cuenca@ie2.u-psud.fr>, gnome-hackers@nuclecu.unam.mx", "James M. Cape <jcape@jcinteractive.com>, Joaquín Cuenca Abela <cuenca@ie2.u-psud.fr>, gnome-hackers@nuclecu.unam.mx" },
	{ 1, "=?ISO-8859-1?Q?David_Guti=E9rrez_Magallanes?= <david@iiia.csic.es>", "David Gutiérrez Magallanes <david@iiia.csic.es>" },
	{ 1, "=?ISO-8859-2?Q?Tomasz_K=B3oczko?= <kloczek@rudy.mif.pg.gda.pl>", "Tomasz Kłoczko <kloczek@rudy.mif.pg.gda.pl>" },
	{ 1, "=?ISO-8859-2?Q?Vladim=EDr_Solnick=FD?= <vs@utia.cas.cz>", "Vladimír Solnický <vs@utia.cas.cz>" },
	{ 1, "=?iso-8859-1?Q?=22S=F6rensen=2C_Daniel=22?= <dasar@wmdata.com>", "\"Sörensen, Daniel\" <dasar@wmdata.com>" },
	{ 1, "=?iso-8859-1?Q?=C1=C2=AAQ=A7=CA?= <dennys@iim.nctu.edu.tw>", "ÁÂªQ§Ê <dennys@iim.nctu.edu.tw>" },
	{ 1, "=?iso-8859-1?Q?=C1=C2=AAQ=A7=CA?= <dennys@news.iim.nctu.edu.tw>", "ÁÂªQ§Ê <dennys@news.iim.nctu.edu.tw>" },
	{ 1, "=?iso-8859-1?Q?=C1kos?= Valentinyi <A.Valentinyi@soton.ac.uk>", "Ákos Valentinyi <A.Valentinyi@soton.ac.uk>" },
	{ 1, "=?iso-8859-1?Q?Joaqu=EDn?= Cuenca Abela <cuenca@ie2.u-psud.fr>", "Joaquín Cuenca Abela <cuenca@ie2.u-psud.fr>" },
	{ 2, "=?iso-8859-1?Q?Joaqu=EDn?= Cuenca Abela <cuenca@ie2.u-psud.fr>, gnome-hackers@nuclecu.unam.mx", "Joaquín Cuenca Abela <cuenca@ie2.u-psud.fr>, gnome-hackers@nuclecu.unam.mx" },
	{ 1, "=?iso-8859-1?Q?Joaqu=EDn_Cuenca_Abela?= <cuenca@celium.net>", "Joaquín Cuenca Abela <cuenca@celium.net>" },
	{ 1, "=?iso-8859-1?Q?Juantom=E1s=20Garc=EDa?= <juantomas@lared.es>", "Juantomás García <juantomas@lared.es>" },
	{ 1, "=?iso-8859-1?Q?Kenneth_ll=E9phaane_Christiansen?= <kenneth@ripen.dk>", "Kenneth lléphaane Christiansen <kenneth@ripen.dk>" },
	{ 1, "=?iso-8859-1?Q?Kjell_Tage_=D8hman?= <tage@ohman.no>", "Kjell Tage Øhman <tage@ohman.no>" },
	{ 1, "=?iso-8859-1?Q?Martin_Norb=E4ck?= <d95mback@dtek.chalmers.se>", "Martin Norbäck <d95mback@dtek.chalmers.se>" },
	{ 1, "=?iso-8859-1?Q?P=E5llen?= <pollen@astrakan.hig.se>", "Pållen <pollen@astrakan.hig.se>" },
	{ 1, "=?iso-8859-1?Q?Ville_P=E4tsi?= <drc@gnu.org>", "Ville Pätsi <drc@gnu.org>" },
	{ 1, "=?iso-8859-1?q?Joaqu=EDn?= Cuenca Abela <cuenca@celium.net>", "Joaquín Cuenca Abela <cuenca@celium.net>" },
	{ 1, "=?iso-8859-2?Q?Dra=BEen_Ka=E8ar?= <dave@srce.hr>", "Dražen Kačar <dave@srce.hr>" },
        /* yep this is right, this isn't valid so doesn't decode at all */
	{ 1, "=?windows-1250?Q? \"Jaka Mo=E8nik\" ?= <jaka.mocnik@kiss.uni-lj.si>", "=?windows-1250?Q? Jaka Mo=E8nik ?= <jaka.mocnik@kiss.uni-lj.si>" },
	{ 3, "George <jirka@5z.com>, Juantomas =?ISO-8859-1?Q?Garc=C3=83=C2=ADa?= <juantomas@lared.es>, gnome-hackers@gnome.org", "George <jirka@5z.com>, Juantomas GarcÃÂ­a <juantomas@lared.es>, gnome-hackers@gnome.org" },
	{ 7, "Jon Trowbridge <trow@emccta.com>, gnome-1.4-list@gnome.org, gnome-devel-list@gnome.org, gnome-hackers@gnome.org, Dom Lachowicz <cinamod@hotmail.com>, =?iso-8859-1?Q?Joaqu=EDn_Cuenca_Abela?= <cuenca@celium.net>, sam th <sam@uchicago.edu>", "Jon Trowbridge <trow@emccta.com>, gnome-1.4-list@gnome.org, gnome-devel-list@gnome.org, gnome-hackers@gnome.org, Dom Lachowicz <cinamod@hotmail.com>, Joaquín Cuenca Abela <cuenca@celium.net>, sam th <sam@uchicago.edu>" },
	{ 6, "Jon Trowbridge <trow@emccta.com>, gnome-1.4-list@gnome.org, gnome-devel-list@gnome.org, gnome-hackers@gnome.org, Dom Lachowicz <cinamod@hotmail.com>, =?iso-8859-1?Q?Joaqu=EDn_Cuenca_Abela?= <cuenca@ie2.u-psud.fr>", "Jon Trowbridge <trow@emccta.com>, gnome-1.4-list@gnome.org, gnome-devel-list@gnome.org, gnome-hackers@gnome.org, Dom Lachowicz <cinamod@hotmail.com>, Joaquín Cuenca Abela <cuenca@ie2.u-psud.fr>" },
	{ 1, "Kai =?iso-8859-1?Q?Gro=DFjohann?= <Kai.Grossjohann@CS.Uni-Dortmund.DE>", "Kai Großjohann <Kai.Grossjohann@CS.Uni-Dortmund.DE>" },
	{ 1, "Kai.Grossjohann@CS.Uni-Dortmund.DE (Kai =?iso-8859-1?q?Gro=DFjohann?=)", "Kai Großjohann <Kai.Grossjohann@CS.Uni-Dortmund.DE>" },
	{ 1, "Rickard =?iso-8859-1?Q?Nordstr=F6m?= <rzi@ebox.tninet.se>", "Rickard Nordström <rzi@ebox.tninet.se>" },
	{ 1, "Tomasz =?iso-8859-2?q?K=B3oczko?= <kloczek@rudy.mif.pg.gda.pl>", "Tomasz Kłoczko <kloczek@rudy.mif.pg.gda.pl>" },
	{ 1, "VALCKE =?iso-8859-1?Q?C=E9dric?= <cvalcke@freesurf.fr>", "VALCKE Cédric <cvalcke@freesurf.fr>" },
	{ 1, "Ville =?iso-8859-1?q?P=E4tsi?= <drc@gnu.org>", "Ville Pätsi <drc@gnu.org>" },
	{ 1, "david@iiia.csic.es (=?ISO-8859-1?Q?David_Guti=E9rrez_Magallanes?=)", "David Gutiérrez Magallanes <david@iiia.csic.es>" },
	{ 1, "kloczek@rudy.mif.pg.gda.pl (=?ISO-8859-2?Q?Tomasz_K=B3oczko?=)", "Tomasz Kłoczko <kloczek@rudy.mif.pg.gda.pl>" },
	{ 1, "lassehp@imv.aau.dk (Lasse =?ISO-8859-1?Q?Hiller=F8e?= Petersen)", "Lasse Hillerøe Petersen <lassehp@imv.aau.dk>" },
	{ 1, "ysato@etl.go.jp (Yutaka Sato =?ISO-2022-JP?B?GyRAOjRGI0stGyhK?=)", "Yutaka Sato 佐藤豊 <ysato@etl.go.jp>" },
};

static struct _l {
    const gchar *type;
    const gchar *line;
} test_lines[] = {
	/* commented out unsupported charsets - FIXME: camel should somehow handle this, although it can't really of course */
	/*{ "windows-1251", "���� ����� �� �������� �� ������ �� �����.\n�� �� �������� �� ��� �� �������?" },*/
	{ "iso-8859-1", "Omple les miniatures de la finestra amb contingut de la pantalla" },
	{ "ISO-8859-2", "Spr�vce oken h�be s okrajem okna\n(AfterStep, Enlightenment, FVWM, IceWM, Sawmill)" },
	{ "ISO-8859-1", "Vinduesh�ndtering flytter dekorationsvindue istedet\n(AfterStep, Enlightenment, FVWM, IceWM, Sawfish)" },
	{ "ISO-8859-1", "Vorschaubilder der Fenster mit dem Bildschirminhalt ausf�llen" },
	{ "iso-8859-7", "�������� �������� ��� �� ��������� ��� ����� ��������� (���������-��������������)" },
	{ "iso-8859-1", "You've chosen to disable the startup hint.\nTo re-enable it, choose \"Startup Hint\"\nin the GNOME Control Centre" },
	{ "iso-8859-1", "El aplique de reloj muestra en su panel la fecha y la hora de forma simple \ny ligero " },
	{ "iso-8859-1", "Applet ei vasta salvestusk�sule.\nKas peaks ta niisama sulgema, v�i veel ootama?" },
	{ "iso-8859-1", "Lehio kudeatzaileak lehioaren dekorazaioa mugiarazten\n(AfterStep, Enlightenment, FVWM, IceWM, Sawmill)" },
	{ "iso-8859-15", "N�yt� sovellukset, joiden ikkunoista on n�kyvill� vain otsikkopalkki" },
	{ "ISO-8859-1", "Afficher les t�ches qui ne sont pas dans la liste des fen�tres" },
	{ "iso-8859-1", "N�l applet ag tabhair freagra ar iarratas s�bh�il.\nBain amach an applet n� lean ar f�nacht?" },
	{ "iso-8859-1", "Amosa-las tarefas agochadas da lista de fiestras (SKIP-WINLIST)" },
	{ "iso-8859-2", "Az ablakkezel� a dekor�ci�t mozgassa az ablak helyett\n(AfterStep, Enlightenment, FVWM, IceWM, SawMill)" },
	{ "iso-8859-1", "Riempi la finestra delle anteprime con il contenuto dello schermo" },
	{ "euc-jp", "������ɥ��ޥ͡�����Ͼ��ꥦ����ɥ���ư����\n(AfterStep, Enlightenment, FVWM, IceWM, Sawfish)" },
	{ "euc-kr", "â �����ڰ� �ٹ� â ��� �̵�\n(AfterStep, Enlightenment, FVWM, IceWM, Sawmill)" },
	{ "iso-8859-13", "Priedas neatsakin�ja � pra�ym� i�sisaugoti.\nPa�alinti pried� ar laukti toliau?" },
	{ "iso-8859-1", "Window manager verplaatst dekoratie\n(AfterStep, Enlightenment, FVWM, IceWM, Sawmill)" },
	{ "iso-8859-1", "Vindush�ndtereren flytter dekorasjonsvinduet i stedet\n(AfterStep, Enlightenment, FVWM, IceWM, Sawfish)" },
	{ "iso-8859-2", "Przemieszczanie dekoracji zamiast okna\n(AfterStep, Enlightenment, FVWM, IceWM, Sawmill)" },
	{ "iso-8859-1", "Este programa � respons�vel por executar outras aplica��es, embeber pequenos applets, a paz no mundo e crashes aleat�rios do X." },
	{ "iso-8859-1", "Mostrar tarefas que se escondem da lista de janelas (SKIP-WINLIST)" },
	{ "koi8-r", "������ �������� ����� � ������������� ��������� � ������� ������" },
	{ "iso-8859-2", "Spr�vca okien pres�va okraje okien\n(AfterStep, Enlightenment, FVWM, IceWM, Sawfish)" },
	{ "iso-8859-2", "Ka�i posle, ki se skrivajo pred upravljalnik oken (SKIP-WINLIST)" },
	{ "iso-8859-5", "Window ��������� ������ ����������� ������ ������ ���a\n(AfterStep, Enlightenment, FVWM, IceWM, Sawmill)" },
	{ "iso-8859-2", "Window menadzeri pomera dekoracioni prozor umesto toga\n(AfterStep, Enlightenment, FVWM, IceWM, Sawmill)" },
	{ "iso-8859-1", "F�nsterhanteraren flyttar dekorationsf�nstret ist�llet\n(AfterStep, Enlightenment, FVWM, IceWM, Sawfish)" },
	/*{ "TSCII", "���츼�-���� ���� ��¡� ���츼���� ���� (���츼�-���-�Ţ�)" },*/
	{ "iso-8859-9", "Kaydetme iste�ine bir uygulak cevap vermiyor .\nUygula�� sileyim mi , yoksa bekleyeyim mi ?" },
	{ "koi8-u", "����ͦ����� ������æ� ��ͦ��� צ���\n(AfterStep, Enlightenment, FVWM, IceWM, Sawfish)" },
	{ "iso-8859-1", "Cwand on scrift�r est bodj� fo�, li scrift�r �t totes\nles apliketes � dvins sont pierdowes. Bodj� ci scrift�r chal?" },
	{ "gb2312", "Ǩ�Ƶ�װ�δ��ڹ�������(AfterStep, Enlightenment, FVWM, IceWM, SawMill)" },
	{ "big5", "�����޲z�̥u���ʸ˹�����\n(AfterStep, Enlightenment, FVWM, IceWM, Sawmill)" },
};

static struct _d {
	const gchar *name;
	const gchar *email;
} test_decode[] = {
	{ NULL,                   "email@example.com" },
	{ NULL,                   "your.email@example.com" },
	{ "Your",                 "email@example.com" },
	{ "Your Email",           "email@example.com" },
	{ "Mr Smith Black",       "smith@black.com" },
	{ "Mr. Smith Black",      "smith@black.com" },
	{ "Mr. Smith O. Black",   "smith.o@black.com" },
	{ "Joe d'Magio",          "joe.d@example.com" },
	{ "example.com address",  "email@example.com" },
	{ "email at example.com", "email@example.com" },
	{ "email.at.example.com", "email@example.com" }
};

static struct _ldf {
	const gchar *with_name;
	const gchar *without_name;
} line_decode_formats[] = {
	{"%s <%s>",      "%s" },
	{ "%s<%s>",      "%s" },
	{ "\"%s\" <%s>", " <%s>"},
	{ "\"%s\"<%s>",  "<%s>"}
};
