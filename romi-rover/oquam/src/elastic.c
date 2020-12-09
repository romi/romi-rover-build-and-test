/*
  romi-rover

  Copyright (C) 2019 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  romi-rover is collection of applications for the Romi Rover.

  romi-rover is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */
// gcc -03 elastic.c -lm  -o elastic
#include <math.h>
#include <string.h>
#include <stdio.h>

typedef double real_t;

typedef struct _point_t {
        real_t x;
        real_t y;
} point_t;

point_t city[114] = {
        { 2.654706588247340501e-02, 2.857010134014911773e-02},
        { 6.623675578545568099e-03, 7.834072697499479265e-02},
        { 8.822466202150946318e-03, 1.390170667516663072e-01},
        { 3.913258885410072704e-03, 2.008077136868911539e-01},
        { 0.000000000000000000e+00, 0.000000000000000000e+00},
        { 2.726300059233391273e-03, 7.053799973971115467e-01},
        { 6.155652302441223279e-03, 7.809308259785316952e-01},
        { 6.518317042207527438e-03, 8.537691343697244584e-01},
        { 7.380784576590530138e-03, 9.239344429872184961e-01},
        { 8.081395867474371747e-03, 9.943172603477886673e-01},
        { 7.039482837908306312e-02, 1.973464973135877296e-03},
        { 7.212816958302736836e-02, 6.472143742891646456e-02},
        { 6.947491955808485220e-02, 1.228091324176631555e-01},
        { 6.478182130504323011e-02, 1.849140945118849799e-01},
        { 5.956777141478258181e-02, 7.599670429986439268e-01},
        { 5.961138399297531004e-02, 8.339702661854025978e-01},
        { 6.441552979929003608e-02, 9.149901001855280658e-01},
        { 6.770400506595164458e-02, 9.912538544984184075e-01},
        { 1.265706045115989886e-01, 8.954504062471353026e-03},
        { 1.299070051934472503e-01, 7.768881302686334034e-02},
        { 1.209327461048517044e-01, 1.623180552391773612e-01},
        { 1.210742210077548064e-01, 7.844494552147771582e-01},
        { 1.089629744585399740e-01, 8.557388196904057720e-01},
        { 1.244373919150612157e-01, 9.219305941739137644e-01},
        { 1.282385930925893924e-01, 9.929261276293819272e-01},
        { 1.639200625263030897e-01, 1.271875730210940136e-01},
        { 1.852804736520764217e-01, 1.914360702600494957e-01},
        { 1.787741511868585487e-01, 7.581813536025016065e-01},
        { 1.614575386102937993e-01, 8.452376007634579969e-01},
        { 1.831595276905436664e-01, 9.248232498536643753e-01},
        { 1.897370296000422396e-01, 1.000000000000000000e+00},
        { 2.585301098409131249e-01, 2.169279289822106105e-01},
        { 2.443310632762895340e-01, 7.411600405079820097e-01},
        { 3.027700617138990302e-01, 2.678810456719177280e-01},
        { 2.936462261120135753e-01, 7.113571366766623383e-01},
        { 3.289239900950693407e-01, 3.249718460021512323e-01},
        { 3.450696199657270480e-01, 3.879156845695032052e-01},
        { 3.703198567757169934e-01, 4.507979647723645567e-01},
        { 3.670148083938734596e-01, 5.869716063360455038e-01},
        { 3.538651310355601654e-01, 6.823436229390720253e-01},
        { 3.908269413662352476e-01, 3.371747418394782092e-01},
        { 4.226653260146733260e-01, 3.905842874611514248e-01},
        { 4.215905167973357792e-01, 4.481396967096547557e-01},
        { 3.859728838053315680e-01, 5.180233776736342666e-01},
        { 4.176832585722014546e-01, 5.778781055980396486e-01},
        { 4.019976295083534379e-01, 6.450975383580622724e-01},
        { 4.148365797191658766e-01, 7.141288766236392194e-01},
        { 4.733583905926518898e-01, 2.000949910056705749e-01},
        { 4.827155559400471141e-01, 2.647689218534861055e-01},
        { 4.565149085795048856e-01, 3.253232917339481944e-01},
        { 4.663594319871993688e-01, 6.009767150490099574e-01},
        { 4.609983014503046261e-01, 6.594318212743413010e-01},
        { 4.742352523797739305e-01, 7.154257586284734627e-01},
        { 4.530100765776263194e-01, 7.680751332793117614e-01},
        { 5.221406213616822667e-01, 5.449103252100087613e-03},
        { 5.364398740235397645e-01, 7.240247881341921321e-02},
        { 5.061490435613731353e-01, 1.438059020057730342e-01},
        { 5.267730722679128119e-01, 2.124788763798495239e-01},
        { 5.268211043690936846e-01, 2.903909150523764637e-01},
        { 5.209779649787943390e-01, 6.424886007779162522e-01},
        { 5.270752898225372629e-01, 7.122852279429487954e-01},
        { 5.116049487950601327e-01, 7.827920776048917784e-01},
        { 5.262691476167734583e-01, 8.472957255191434500e-01},
        { 5.437110411893975037e-01, 9.219041192897258208e-01},
        { 5.277466430071945380e-01, 9.942096820255700296e-01},
        { 5.834496890748285258e-01, 6.954403851590636176e-03},
        { 5.903181363125671322e-01, 6.993183043168921253e-02},
        { 5.581676573477436509e-01, 1.391206726650930925e-01},
        { 5.776947837424661891e-01, 2.130563653453434181e-01},
        { 5.770409973720758146e-01, 2.693522599664946449e-01},
        { 5.781488568860201349e-01, 6.374261253479269751e-01},
        { 5.813266228800753055e-01, 7.078486923148460397e-01},
        { 5.730845020660909617e-01, 7.768712318457223764e-01},
        { 5.727035510630992254e-01, 8.521191472695966995e-01},
        { 5.890158534873510821e-01, 9.963881250210531171e-01},
        { 6.385890491743199116e-01, 2.674940579486721481e-01},
        { 6.368975798975049507e-01, 6.351147567977162289e-01},
        { 6.357647127807054543e-01, 7.038058981000557823e-01},
        { 6.386305371933733710e-01, 7.700623812413399660e-01},
        { 6.218225888463827511e-01, 8.212589069139224485e-01},
        { 6.968874483601783520e-01, 3.080919889656462130e-01},
        { 6.926167137051183476e-01, 6.240932949532852492e-01},
        { 6.928088323117190805e-01, 6.891139031345496768e-01},
        { 6.907686089199025492e-01, 7.535690700329037428e-01},
        { 7.501898415412651122e-01, 3.194195957304115652e-01},
        { 7.434773831266119348e-01, 3.844661204260127474e-01},
        { 7.517096581938715882e-01, 4.530120585175846926e-01},
        { 7.627414910851717522e-01, 5.238715564853629880e-01},
        { 7.417173641670670170e-01, 5.879762303788850231e-01},
        { 7.490662953648434819e-01, 6.544148510291684095e-01},
        { 7.482740590440453143e-01, 7.212144316901155650e-01},
        { 8.015160118103966669e-01, 3.343956935568836442e-01},
        { 8.088420883595204280e-01, 4.013298440771160824e-01},
        { 8.015675192247870839e-01, 4.674307630419144433e-01},
        { 8.269326570991889502e-01, 5.276314660658723010e-01},
        { 8.000295005580271290e-01, 5.894327491078561465e-01},
        { 8.026111112017667581e-01, 6.575736870445603177e-01},
        { 8.115699727375074168e-01, 7.230516959235691044e-01},
        { 8.552294293441272677e-01, 3.541084711586597322e-01},
        { 8.804858931565295865e-01, 3.952704770039373594e-01},
        { 8.540029784837146476e-01, 4.579163289086347621e-01},
        { 8.601852514738829170e-01, 5.910885112627960458e-01},
        { 8.532893838991711943e-01, 6.487499934422042980e-01},
        { 8.665820546958090675e-01, 7.029788755926412724e-01},
        { 8.743120348455972746e-01, 7.594473801562012794e-01},
        { 9.168135935883416909e-01, 3.324541279870942834e-01},
        { 9.052741045124321762e-01, 6.376716068756974609e-01},
        { 9.260703149697653291e-01, 7.034940053298273099e-01},
        { 9.293278086506441271e-01, 7.798071076970698456e-01},
        { 9.804561541736374064e-01, 2.841391629574180344e-01},
        { 9.765149753302684310e-01, 7.233609467644785962e-01},
        { 9.804987379304112016e-01, 7.792729090848810047e-01},
        { 9.851735953065300366e-01, 8.282554371665153958e-01},
        { 1.000000000000000000e+00, 8.894871514467422857e-01}
};
        
int num_cities = 114;

point_t path[285] = {
        { 5.786003712441580848e-01, 5.337358602617415793e-01},
        { 5.785758989126823515e-01, 5.359480689770602657e-01},
        { 5.785024938961553786e-01, 5.381591949342794390e-01},
        { 5.783801921224152043e-01, 5.403681559052520011e-01},
        { 5.782090534516525215e-01, 5.425738707214757817e-01},
        { 5.779891616471131144e-01, 5.447752598032685745e-01},
        { 5.777206243340995417e-01, 5.469712456881637985e-01},
        { 5.774035729472944967e-01, 5.491607535582717636e-01},
        { 5.770381626664305985e-01, 5.513427117663451948e-01},
        { 5.766245723403383705e-01, 5.535160523602933313e-01},
        { 5.761630043994087069e-01, 5.556797116058875829e-01},
        { 5.756536847565147941e-01, 5.578326305074030600e-01},
        { 5.750968626964390040e-01, 5.599737553259395151e-01},
        { 5.744928107538620488e-01, 5.621020380951712303e-01},
        { 5.738418245799712381e-01, 5.642164371342702767e-01},
        { 5.731442227977551207e-01, 5.663159175577533455e-01},
        { 5.724003468460547861e-01, 5.683994517820034620e-01},
        { 5.716105608124476545e-01, 5.704660200282172244e-01},
        { 5.707752512550459123e-01, 5.725146108215317664e-01},
        { 5.698948270132969673e-01, 5.745442214860880803e-01},
        { 5.689697190078784050e-01, 5.765538586357871198e-01},
        { 5.680003800297855898e-01, 5.785425386604994280e-01},
        { 5.669872845187141630e-01, 5.805092882074898153e-01},
        { 5.659309283308473493e-01, 5.824531446578212757e-01},
        { 5.648318284961608704e-01, 5.843731565975059938e-01},
        { 5.636905229653642602e-01, 5.862683842831715175e-01},
        { 5.625075703466029253e-01, 5.881379001020148323e-01},
        { 5.612835496320489614e-01, 5.899807890258196297e-01},
        { 5.600190599145155046e-01, 5.917961490588136142e-01},
        { 5.587147200942328418e-01, 5.935830916791473566e-01},
        { 5.573711685759299428e-01, 5.953407422737779786e-01},
        { 5.559890629563687403e-01, 5.970682405665462822e-01},
        { 5.545690797024865892e-01, 5.987647410392352709e-01},
        { 5.531119138203008934e-01, 6.004294133454066706e-01},
        { 5.516182785147414336e-01, 6.020614427168109462e-01},
        { 5.500889048405742665e-01, 6.036600303621729724e-01},
        { 5.485245413445895002e-01, 6.052243938581578497e-01},
        { 5.469259536992273629e-01, 6.067537675323250168e-01},
        { 5.452939243278230874e-01, 6.082474028378843656e-01},
        { 5.436292520216516877e-01, 6.097045687200700614e-01},
        { 5.419327515489626990e-01, 6.111245519739523235e-01},
        { 5.402052532561945064e-01, 6.125066575935134150e-01},
        { 5.384476026615637734e-01, 6.138502091118164250e-01},
        { 5.366606600412301420e-01, 6.151545489320989768e-01},
        { 5.348453000082361575e-01, 6.164190386496324336e-01},
        { 5.330024110844312490e-01, 6.176430593641863975e-01},
        { 5.311328952655879343e-01, 6.188260119829478434e-01},
        { 5.292376675799224106e-01, 6.199673175137443426e-01},
        { 5.273176556402376924e-01, 6.210664173484308215e-01},
        { 5.253737991899062321e-01, 6.221227735362977462e-01},
        { 5.234070496429159558e-01, 6.231358690473691730e-01},
        { 5.214183696182035366e-01, 6.241052080254619883e-01},
        { 5.194087324685044971e-01, 6.250303160308804395e-01},
        { 5.173791218039482942e-01, 6.259107402726293845e-01},
        { 5.153305310106337522e-01, 6.267460498300312377e-01},
        { 5.132639627644199898e-01, 6.275358358636383693e-01},
        { 5.111804285401697623e-01, 6.282797118153385929e-01},
        { 5.090809481166866934e-01, 6.289773135975547103e-01},
        { 5.069665490775877581e-01, 6.296282997714456320e-01},
        { 5.048382663083559319e-01, 6.302323517140225873e-01},
        { 5.026971414898194768e-01, 6.307891737740982663e-01},
        { 5.005442225883041107e-01, 6.312984934169922902e-01},
        { 4.983805633427098036e-01, 6.317600613579218427e-01},
        { 4.962072227487616671e-01, 6.321736516840141817e-01},
        { 4.940252645406882914e-01, 6.325390619648779689e-01},
        { 4.918357566705803263e-01, 6.328561133516830139e-01},
        { 4.896397707856850468e-01, 6.331246506646966976e-01},
        { 4.874383817038923095e-01, 6.333445424692361048e-01},
        { 4.852326668876684734e-01, 6.335156811399986765e-01},
        { 4.830237059166959668e-01, 6.336379829137389619e-01},
        { 4.808125799594767380e-01, 6.337113879302658237e-01},
        { 4.786003712441580515e-01, 6.337358602617415571e-01},
        { 4.763881625288393651e-01, 6.337113879302658237e-01},
        { 4.741770365716201363e-01, 6.336379829137389619e-01},
        { 4.719680756006476852e-01, 6.335156811399986765e-01},
        { 4.697623607844237936e-01, 6.333445424692361048e-01},
        { 4.675609717026311118e-01, 6.331246506646966976e-01},
        { 4.653649858177358323e-01, 6.328561133516830139e-01},
        { 4.631754779476278117e-01, 6.325390619648779689e-01},
        { 4.609935197395544360e-01, 6.321736516840141817e-01},
        { 4.588201791456063550e-01, 6.317600613579218427e-01},
        { 4.566565199000119923e-01, 6.312984934169922902e-01},
        { 4.545036009984966263e-01, 6.307891737740982663e-01},
        { 4.523624761799601712e-01, 6.302323517140225873e-01},
        { 4.502341934107284005e-01, 6.296282997714456320e-01},
        { 4.481197943716294096e-01, 6.289773135975547103e-01},
        { 4.460203139481463408e-01, 6.282797118153385929e-01},
        { 4.439367797238961133e-01, 6.275358358636383693e-01},
        { 4.418702114776823509e-01, 6.267460498300312377e-01},
        { 4.398216206843678644e-01, 6.259107402726293845e-01},
        { 4.377920100198115505e-01, 6.250303160308804395e-01},
        { 4.357823728701125110e-01, 6.241052080254619883e-01},
        { 4.337936928454002028e-01, 6.231358690473691730e-01},
        { 4.318269432984098710e-01, 6.221227735362977462e-01},
        { 4.298830868480784106e-01, 6.210664173484308215e-01},
        { 4.279630749083936925e-01, 6.199673175137443426e-01},
        { 4.260678472227281688e-01, 6.188260119829478434e-01},
        { 4.241983314038847985e-01, 6.176430593641863975e-01},
        { 4.223554424800800011e-01, 6.164190386496324336e-01},
        { 4.205400824470859611e-01, 6.151545489320989768e-01},
        { 4.187531398267523297e-01, 6.138502091118164250e-01},
        { 4.169954892321216522e-01, 6.125066575935134150e-01},
        { 4.152679909393533486e-01, 6.111245519739523235e-01},
        { 4.135714904666643599e-01, 6.097045687200700614e-01},
        { 4.119068181604929602e-01, 6.082474028378843656e-01},
        { 4.102747887890886846e-01, 6.067537675323250168e-01},
        { 4.086762011437266584e-01, 6.052243938581578497e-01},
        { 4.071118376477418366e-01, 6.036600303621729724e-01},
        { 4.055824639735746140e-01, 6.020614427168109462e-01},
        { 4.040888286680152097e-01, 6.004294133454066706e-01},
        { 4.026316627858295694e-01, 5.987647410392352709e-01},
        { 4.012116795319473628e-01, 5.970682405665462822e-01},
        { 3.998295739123862158e-01, 5.953407422737779786e-01},
        { 3.984860223940832613e-01, 5.935830916791473566e-01},
        { 3.971816825738006540e-01, 5.917961490588136142e-01},
        { 3.959171928562671972e-01, 5.899807890258196297e-01},
        { 3.946931721417131778e-01, 5.881379001020148323e-01},
        { 3.935102195229518429e-01, 5.862683842831715175e-01},
        { 3.923689139921552327e-01, 5.843731565975059938e-01},
        { 3.912698141574687538e-01, 5.824531446578212757e-01},
        { 3.902134579696019401e-01, 5.805092882074898153e-01},
        { 3.892003624585305133e-01, 5.785425386604994280e-01},
        { 3.882310234804376980e-01, 5.765538586357871198e-01},
        { 3.873059154750191913e-01, 5.745442214860880803e-01},
        { 3.864254912332701908e-01, 5.725146108215317664e-01},
        { 3.855901816758684486e-01, 5.704660200282172244e-01},
        { 3.848003956422613170e-01, 5.683994517820034620e-01},
        { 3.840565196905610379e-01, 5.663159175577533455e-01},
        { 3.833589179083449205e-01, 5.642164371342702767e-01},
        { 3.827079317344540543e-01, 5.621020380951712303e-01},
        { 3.821038797918770435e-01, 5.599737553259395151e-01},
        { 3.815470577318013090e-01, 5.578326305074030600e-01},
        { 3.810377380889073962e-01, 5.556797116058875829e-01},
        { 3.805761701479777326e-01, 5.535160523602933313e-01},
        { 3.801625798218855046e-01, 5.513427117663451948e-01},
        { 3.797971695410216619e-01, 5.491607535582718747e-01},
        { 3.794801181542165613e-01, 5.469712456881637985e-01},
        { 3.792115808412029887e-01, 5.447752598032685745e-01},
        { 3.789916890366635815e-01, 5.425738707214758927e-01},
        { 3.788205503659009543e-01, 5.403681559052520011e-01},
        { 3.786982485921606689e-01, 5.381591949342794390e-01},
        { 3.786248435756337516e-01, 5.359480689770602657e-01},
        { 3.786003712441580182e-01, 5.337358602617415793e-01},
        { 3.786248435756337516e-01, 5.315236515464228928e-01},
        { 3.786982485921606689e-01, 5.293125255892037195e-01},
        { 3.788205503659009543e-01, 5.271035646182312684e-01},
        { 3.789916890366635815e-01, 5.248978498020073769e-01},
        { 3.792115808412029887e-01, 5.226964607202145841e-01},
        { 3.794801181542165613e-01, 5.205004748353193600e-01},
        { 3.797971695410216064e-01, 5.183109669652113949e-01},
        { 3.801625798218855046e-01, 5.161290087571379637e-01},
        { 3.805761701479777326e-01, 5.139556681631898272e-01},
        { 3.810377380889073407e-01, 5.117920089175955756e-01},
        { 3.815470577318013090e-01, 5.096390900160800985e-01},
        { 3.821038797918770435e-01, 5.074979651975436434e-01},
        { 3.827079317344540543e-01, 5.053696824283119282e-01},
        { 3.833589179083449205e-01, 5.032552833892128819e-01},
        { 3.840565196905609824e-01, 5.011558029657298130e-01},
        { 3.848003956422613170e-01, 4.990722687414796965e-01},
        { 3.855901816758683931e-01, 4.970057004952659341e-01},
        { 3.864254912332701908e-01, 4.949571097019513921e-01},
        { 3.873059154750191913e-01, 4.929274990373950782e-01},
        { 3.882310234804376425e-01, 4.909178618876960387e-01},
        { 3.892003624585304578e-01, 4.889291818629837305e-01},
        { 3.902134579696019401e-01, 4.869624323159933987e-01},
        { 3.912698141574687538e-01, 4.850185758656619384e-01},
        { 3.923689139921552327e-01, 4.830985639259772202e-01},
        { 3.935102195229518429e-01, 4.812033362403117520e-01},
        { 3.946931721417131778e-01, 4.793338204214683262e-01},
        { 3.959171928562671416e-01, 4.774909314976635288e-01},
        { 3.971816825738005985e-01, 4.756755714646695443e-01},
        { 3.984860223940832058e-01, 4.738886288443358574e-01},
        { 3.998295739123862158e-01, 4.721309782497051799e-01},
        { 4.012116795319473628e-01, 4.704034799569368763e-01},
        { 4.026316627858295139e-01, 4.687069794842478876e-01},
        { 4.040888286680152097e-01, 4.670423071780764879e-01},
        { 4.055824639735746140e-01, 4.654102778066722124e-01},
        { 4.071118376477417811e-01, 4.638116901613101861e-01},
        { 4.086762011437266029e-01, 4.622473266653253088e-01},
        { 4.102747887890886846e-01, 4.607179529911581417e-01},
        { 4.119068181604929602e-01, 4.592243176855987374e-01},
        { 4.135714904666643599e-01, 4.577671518034130971e-01},
        { 4.152679909393533486e-01, 4.563471685495308905e-01},
        { 4.169954892321215967e-01, 4.549650629299697435e-01},
        { 4.187531398267523297e-01, 4.536215114116667890e-01},
        { 4.205400824470859611e-01, 4.523171715913841817e-01},
        { 4.223554424800799456e-01, 4.510526818738507249e-01},
        { 4.241983314038847985e-01, 4.498286611592967055e-01},
        { 4.260678472227281133e-01, 4.486457085405353706e-01},
        { 4.279630749083936370e-01, 4.475044030097388159e-01},
        { 4.298830868480783551e-01, 4.464053031750523370e-01},
        { 4.318269432984098155e-01, 4.453489469871854678e-01},
        { 4.337936928454002583e-01, 4.443358514761139855e-01},
        { 4.357823728701125665e-01, 4.433665124980211703e-01},
        { 4.377920100198115505e-01, 4.424414044926027190e-01},
        { 4.398216206843678644e-01, 4.415609802508537185e-01},
        { 4.418702114776823509e-01, 4.407256706934519208e-01},
        { 4.439367797238961133e-01, 4.399358846598448447e-01},
        { 4.460203139481462853e-01, 4.391920087081445656e-01},
        { 4.481197943716293541e-01, 4.384944069259284483e-01},
        { 4.502341934107283450e-01, 4.378434207520375820e-01},
        { 4.523624761799601157e-01, 4.372393688094605713e-01},
        { 4.545036009984965708e-01, 4.366825467493848367e-01},
        { 4.566565199000119368e-01, 4.361732271064909239e-01},
        { 4.588201791456062995e-01, 4.357116591655613158e-01},
        { 4.609935197395543804e-01, 4.352980688394690323e-01},
        { 4.631754779476277561e-01, 4.349326585586051896e-01},
        { 4.653649858177358323e-01, 4.346156071718000891e-01},
        { 4.675609717026311118e-01, 4.343470698587865164e-01},
        { 4.697623607844237936e-01, 4.341271780542471093e-01},
        { 4.719680756006476852e-01, 4.339560393834844820e-01},
        { 4.741770365716201363e-01, 4.338337376097441966e-01},
        { 4.763881625288393651e-01, 4.337603325932172793e-01},
        { 4.786003712441580515e-01, 4.337358602617416015e-01},
        { 4.808125799594767380e-01, 4.337603325932172793e-01},
        { 4.830237059166959113e-01, 4.338337376097441966e-01},
        { 4.852326668876684179e-01, 4.339560393834844820e-01},
        { 4.874383817038922539e-01, 4.341271780542470538e-01},
        { 4.896397707856849912e-01, 4.343470698587865164e-01},
        { 4.918357566705802153e-01, 4.346156071718000891e-01},
        { 4.940252645406882359e-01, 4.349326585586051896e-01},
        { 4.962072227487616671e-01, 4.352980688394690323e-01},
        { 4.983805633427098036e-01, 4.357116591655613158e-01},
        { 5.005442225883041107e-01, 4.361732271064908684e-01},
        { 5.026971414898194768e-01, 4.366825467493848367e-01},
        { 5.048382663083559319e-01, 4.372393688094605713e-01},
        { 5.069665490775877581e-01, 4.378434207520375265e-01},
        { 5.090809481166866934e-01, 4.384944069259284483e-01},
        { 5.111804285401697623e-01, 4.391920087081445656e-01},
        { 5.132639627644199898e-01, 4.399358846598447892e-01},
        { 5.153305310106337522e-01, 4.407256706934519208e-01},
        { 5.173791218039481832e-01, 4.415609802508537185e-01},
        { 5.194087324685044971e-01, 4.424414044926026635e-01},
        { 5.214183696182035366e-01, 4.433665124980211703e-01},
        { 5.234070496429158448e-01, 4.443358514761139855e-01},
        { 5.253737991899062321e-01, 4.453489469871854678e-01},
        { 5.273176556402376924e-01, 4.464053031750523370e-01},
        { 5.292376675799224106e-01, 4.475044030097388159e-01},
        { 5.311328952655879343e-01, 4.486457085405353706e-01},
        { 5.330024110844312490e-01, 4.498286611592967055e-01},
        { 5.348453000082360465e-01, 4.510526818738507249e-01},
        { 5.366606600412301420e-01, 4.523171715913841262e-01},
        { 5.384476026615637734e-01, 4.536215114116667335e-01},
        { 5.402052532561943954e-01, 4.549650629299697435e-01},
        { 5.419327515489626990e-01, 4.563471685495308905e-01},
        { 5.436292520216516877e-01, 4.577671518034130416e-01},
        { 5.452939243278230874e-01, 4.592243176855986819e-01},
        { 5.469259536992273629e-01, 4.607179529911581417e-01},
        { 5.485245413445893892e-01, 4.622473266653253088e-01},
        { 5.500889048405742665e-01, 4.638116901613101861e-01},
        { 5.516182785147414336e-01, 4.654102778066722124e-01},
        { 5.531119138203008934e-01, 4.670423071780764879e-01},
        { 5.545690797024865892e-01, 4.687069794842478876e-01},
        { 5.559890629563687403e-01, 4.704034799569368763e-01},
        { 5.573711685759299428e-01, 4.721309782497051799e-01},
        { 5.587147200942328418e-01, 4.738886288443358019e-01},
        { 5.600190599145155046e-01, 4.756755714646694888e-01},
        { 5.612835496320489614e-01, 4.774909314976634733e-01},
        { 5.625075703466029253e-01, 4.793338204214683262e-01},
        { 5.636905229653642602e-01, 4.812033362403116410e-01},
        { 5.648318284961608704e-01, 4.830985639259771647e-01},
        { 5.659309283308473493e-01, 4.850185758656618829e-01},
        { 5.669872845187141630e-01, 4.869624323159933432e-01},
        { 5.680003800297855898e-01, 4.889291818629836750e-01},
        { 5.689697190078784050e-01, 4.909178618876960387e-01},
        { 5.698948270132969673e-01, 4.929274990373950782e-01},
        { 5.707752512550459123e-01, 4.949571097019513921e-01},
        { 5.716105608124476545e-01, 4.970057004952658786e-01},
        { 5.724003468460547861e-01, 4.990722687414796410e-01},
        { 5.731442227977551207e-01, 5.011558029657298130e-01},
        { 5.738418245799712381e-01, 5.032552833892128819e-01},
        { 5.744928107538620488e-01, 5.053696824283119282e-01},
        { 5.750968626964390040e-01, 5.074979651975436434e-01},
        { 5.756536847565147941e-01, 5.096390900160800985e-01},
        { 5.761630043994087069e-01, 5.117920089175954645e-01},
        { 5.766245723403383705e-01, 5.139556681631898272e-01},
        { 5.770381626664305985e-01, 5.161290087571378526e-01},
        { 5.774035729472943856e-01, 5.183109669652112839e-01},
        { 5.777206243340995417e-01, 5.205004748353193600e-01},
        { 5.779891616471131144e-01, 5.226964607202145841e-01},
        { 5.782090534516525215e-01, 5.248978498020073769e-01},
        { 5.783801921224152043e-01, 5.271035646182311574e-01},
        { 5.785024938961553786e-01, 5.293125255892037195e-01},
        { 5.785758989126823515e-01, 5.315236515464228928e-01},
        { 5.786003712441580848e-01, 5.337358602617415793e-01}
};

int path_size = 285;

point_t forces[285];

real_t ws[285][114];
real_t sum[114];

#define ka 0.99
#define alpha 0.2
#define beta 0.2

real_t k;
real_t K;

#define NE 4096
#define KE 8.0
real_t _e[NE];

static inline real_t _exp(real_t x)
{
        real_t e = 0.0;
        int i = (int) (-NE * x / KE);
        if (i < NE)
                e = _e[i];
        //printf("x=%f, i=%d, e=%f\n", x, i, e);
        return e;
}

void update_distance_forces()
{
        for (int i = 0; i < path_size; i++)  {
                for (int j = 0; j < num_cities; j++)  {
                        real_t dx = city[j].x - path[i].x;
                        real_t dy = city[j].y - path[i].y;
                        real_t d = dx * dx + dy * dy;
                        ws[i][j] = _exp(K * d);
                        sum[j] += ws[i][j];
                }
        }

        /* for (int j = 0; j < num_cities; j++) */
        /*         sum[j] += 0.00001; */
        
        for (int i = 0; i < path_size; i++) {
                for (int j = 0; j < num_cities; j++) {
                        real_t dx = city[j].x - path[i].x;
                        real_t dy = city[j].y - path[i].y;
                        //real_t w = alpha * ws[i][j] / sum[j];
                        real_t w = alpha * ws[i][j] / (sum[j] + 0.00001);
                        forces[i].x += dx * w;
                        forces[i].y += dy * w;
                }
        }
}

static inline real_t tension(int i, int j)
{
        real_t dx = path[j].x - path[i].x;
        real_t dy = path[j].y - path[i].y;
        forces[i].x += beta * dx;
        forces[i].y += beta * dy;
}

void update_tension()
{
        tension(0, 1);
        tension(0, 113);

        for (int i = 1; i < path_size - 1; i++)  {
                tension(i, i-1);
                tension(i, i+1);                
        }
        
        tension(113, 112);
        tension(113, 0);
}

void update_forces()
{
        memset(forces, 0, sizeof(forces));
        memset(sum, 0, sizeof(sum));
        update_distance_forces();
        update_tension();
}

void update_positions()
{
        for (int i = 0; i < path_size; i++) {
                path[i].x += forces[i].x;
                path[i].y += forces[i].y;
        }
}

void print_positions()
{
        for (int i = 0; i < path_size; i++)
                printf("%f\t%f\n", path[i].x, path[i].y);
}

void print_sum()
{
        for (int i = 0; i < num_cities; i++)
                printf("%f\t\n", sum[i]);
}

int main(int argc, char **argv)
{
        for (int i = 0; i < NE; i++)
                _e[i] = exp(-KE * (real_t) i / NE);

        k = 0.2;
        K = -1.0 / (2.0 * k * k);

        for (int n = 0; n < 7000; n++) {
                
                update_forces();
                update_positions();

                if ((n % 25) == 0) {
                        real_t newk = k * ka;
                        if (newk < 0.01)
                                k = 0.01;
                        else 
                                k = newk;
                        K = -1.0 / (2.0 * k * k);
                }
        }

        /* printf("sum\n"); */
        /* print_sum(); */
        
        /* printf("positions\n"); */
        print_positions();
}