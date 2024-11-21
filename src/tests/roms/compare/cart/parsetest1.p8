pico-8 cartridge // http://www.pico-8.com
version 32
__lua__
printh("SPRITE SHEET")
for i=0x0000,0x0FFF do
    printh(tostr(@i, true))
end

printh("SHARED SPRITE SHEET / MAP")
for i=0x1000,0x1FFF do
    printh(tostr(@i, true))
end

printh("MAP")
for i=0x2000,0x2FFF do
    printh(tostr(@i, true))
end

printh("SPRITE FLAGS")
for i=0x3000,0x30FF do
    printh(tostr(@i, true))
end

printh("MUSIC")
for i=0x3100,0x31FF do
    printh(tostr(@i, true))
end

printh("SOUND EFFECTS")
for i=0x3200,0x42FF do
    printh(tostr(@i, true))
end

printh("====DONE====")

__gfx__
30877432d1026706d7e805da846a32c3bb81e3c29b62179273c8eb5bb682575ec87a171ac826a6fce48478dcb74f21345d2cce8038a39d5e0853964b50af03b9
71722f244f58d669cbee3772a077021721a278f64f7fd633dbdde131ca3766e4d58e72e310275dff6c15c0c8e9df469611a11f5125227c3712da86a78c49ea20
e32684b27b95e909348334896a68f812d810a485ed03241b4d419b1b673bd4755d05ad7853c1f76eb97706ca828bca0385813dbad3c681d06bd2aa399dac946d
c59c0996daeee6f529a279764017f2ed6cfc7403d75e173e4eaede5fe878f78e2978aa2447c462ddaed16dc0cf0b9cd7f78df0cac5e40c02d4e518ca6eaac8d8
2f01b7210760474f36e8b5359309cc6273931bdb2a0df3dbe4d58fed8a728e7eca0fa5f6b8a880627df7ffe0297c79bfbdabe898736a3566f893697b59048119
4f309ffea518f32cf21449273d7cee9d9136682575250def91799e2786d3748421599e3e9c8fe21da80270815fe85df2fbdaa35adf9c1e2a8a3c0ed16bfe1684
9ef307590d273e34f98dff7e4c6428da8099f4efbacea67c7d1afcc4f14a3e3e04d42f8ac2acaf127972d33e5901a19bbd47d5552c7f47e8e80e952eb9d8e96c
f37cb990c801f97b7684319e1b429ad564b858f9a3e247cb2c083eb8cb37f0a72e9d34119f3374cebd4d3fd81b6ee7b3bb1c863e2601a7462667a40844853040
b7a05814d32feb3e719e01fcd3fe22a4248ac9ed336de7daecd3ada8b4f2222d3b41a3dbd199b364f73bb387d080589ab054c24026cdea5b9a2145128edfed86
3bd39f917c10696489a30fd54c7b2c1d0e2adcd93c0a5eb2d37dc2c9a7a5236bb4734865425feeaa4e2fe981b29ee11b922ce1e6af41e3a2517ee5bb9cda1a2a
3c984a24b9c429ca42db0b956af67442931a4c4555e1db7e9e779f6bee9cd56481fb339258e4d27eb0d1cb7c2b70a3a4419f4fe020864d3979317de23f0749d0
b7d52b20cf1cb80b2b73a41ba5ef542e196161a9cf8169b1a83bdceca5ffb82d2d59a32a99ed5ebe1bd812cb504e1427bbc14ebbe24bca87305fc388e69f6342
e5e2ab29955b73647f0bbe4229cfdd24a2eeb454d134955a7b92868492545a102186d0f99f7c9e215edfe6a4aabc4b3a7e38e74319cd75aa65fef9f02ce76b11
9ff903d48bcb1c16b92ce8343cbab46c1114afe44aa5c9af9f0ba3d90f871f5c471360ead4d6df146afca5eab8f67897996fafb893ccb49192be8f6688437717
713daf3405dff69a912715d51cf591093a9ef4e863a5e850a965cda2c354fa708c7e8a908b713e95c939b774f4ebdf672eb231645ae36f2e1e4de1e90c80621d
b212f19d54dbcecc24b35c47009edc77eb48631d076231e171ce761497aa7947d9815df1bcadd49c5f7794e1dd4c786a2eb2618c1266f6a90663f76c7a9ceb98
bfe3fa6bad17408d946a7c7fa8ffe5b54f511210d472406eb1ff00d00890d5334768b8c2bce779212cccf1052fda3176f812815a064c2957cac42b13d72aca08
ef7bcd5c2972284c4cab3209eb83425ded302b2ac09dc275c54898f425d8d9f2b87f6e3490cacaead49a6fa5ca9f7ac8cb3650e6e92df49784dc2efcd1b237b5
1cad303877ebce4b0f39d234b9ae6fbf3eea29130a35755ade7c55dc06edc0668235ba6e38facc3bbe5924a37935b4cd4cd5f55f945ae1b0f46cfdfdef520791
8795ef338b1e6d3791e8b2e376bd54661b85a99834d184474a7cf48dce22c8befa02eb2c6d6f8a9a4fa113e035ee0d649582b82b51c97d2306f247e00a3d4f27
c233ab94c44205eb64de62343cbda4782790966c917fc37f20ba4cdb5f20208611c9ddc24829264ac29d7172d3e19530405fb85b4830ad8282feb1f5b5833701
071fbc451d7a7da82b31571c2e99a2e0b6997ebf6740d07b0a0c9367df148217dbe234c21d4798acaae872643435eead3b6e9e8325916a427bc19850ce73e343
01746cb282026e42a31e15dcf0cd5b6588e4179fdf128c4d670cbffbac850a7081fb75377817cb557ab0b46f95f121770f0a64a5a10443b2bc3a9a45dfa5b75c
99450c15a73f4a27ba52ae08672b8301ced5dfcbc3f75e2190a832a5c522af0d5d513a66d899731cf41b0d29f6306592f39cff82c5bcb5e18ee8781432bd71cd
f7f92c143e556641d2d648a22cca8e0d3d443339bd8cff158c4c1ca71f8b0a998f3749ea8d26e6dfb1529c40566171e1b68bec307bfe5fbb58290c1567768d00
f4507898dcbe86e9c30b993f2a8a8896471ca40f98dcc16a7fb95593f485a27b79dab89e3f12f63c9d1446ade4a52fa5a10e8655f24ddcdfc016b0a60077b943
c952199ead4afb65c07746053b1c8113013dec38f4609d384d33933f6686bd951f6fa70023f422387e98e13519bad331045abe82ba53cce8cfd534153dfe5cb0
4ff3de128a07a3d7fbc4105ff52fa7a817cc72eee2fea3f03cd10296eab17eafbe3370ab9b315f4d38663c6e6a3d13ee4f01df5543cacd78ca9e44d9a6669b45
a3bfd9d030c4116859841961be37c791ccda1086e7b669e52553c1d884580ae414a19fb2a7525dc2b76aab96f03be771ac3c890bef196a2350266d36d240ea12
2158278dcecda0c30212b39929ecc0f574c949b04310c296b6d455786351e292836fab473926afea94bad50a77d8b4afeeb9f35284682200c618f4bc794e2cb0
754e554fb17f728b716bcfe11a3885ccb28c7cbbff04e57286455b37da3fff65d071454141585c0926eff57d4585ae27cc4306d435f132f40ddb1d7fcb3d48f7
29d860030c6adb34d88db8c6df5bf89bc437e536ca15c024fd2287b21cc915fe06961751b70528cbcc60229bb876ec085d329a388ecf7aee0f382c77adb08792
ca25fab6856f67786767b4332f01fbaf8f58c741df1bc5e3ea006c3ab85878fab5fd6dbbc8e547387dc644f05df4af981c35168f3ea8bb8b0d3b659bafe2c9e4
5adc225a7aa98c8ebed550478265c332f10c23842c9779e44501bafe8e45ed9bf72e9bd849004b9f0ff90d970b6ddc75cc782d7898d625493ee8f6a041053984
e07240f6ad9fbe1a2418c2f568c037ce716e36fc9a5138f96b1637da0583c701f4b275f2a11b434f7abe60cb481fe9f65bae8524e98be0c50b7a2c6f49ada332
145163f631cf81b7206f2e1bdb1812926337c6675d3bed355ca5ebaabdda76c8beec0190490976a08431eb448b77892c62af5f391c21abdd370c191a4a741ce2
7d9c44a2f1c82cd44f6fc67728da23ddbb6ab095be4e176b42317490a39ef0a6668f40c18519681e02c8b309c7c3af256e0179afc50bbb97818c0874ac42c7d7
4d9ae4646494d45a235a40add9e846345087770b2f4fb5cff45671d08d76625efae7dc1cac13ee17c1c169ec99e5d914ee2354cdae05e6e28a5323eb2c5cc15a
45451d99e95346080eff0f76fede207861541b1419a213d5595eb129abc2d29f438ad66132f9da8b4fff5796030e36dd1ab60698299a03aac056aaff14f4eaed
19a06ab2480ac5c539a18d2f7be96953b162e0f46af9a43461ec30912ae139096a6698ae384583036ba8497529ae140f13c12dc5eb9a62e42e3e9ef7748bc5aa
ef02f3bfe59b43c3a29ecd775fc2a6dda752f3ea3e59c23caf1264044e9ce66a99db20c491b10b3907dcacccd65f46cbd49440204fd424ded5edecb75d0f78db
11fb3f248e227f291a0fefadb9951981f51909f2428848880354eb587f51a244fdc7e56b18315ecf9f7ccf84d09eca13bd8a8838ce76a5a0020d33eb79861021
63324c53589e2e8da85281cca1885e2f6c5f34d63e831228e0f401c84ac0ffdc270cf3ba9c12ba2e9651c69c3bf2e641607fc29fe01a1a1c36e47214f0f17405
193e5233f726daca34a615a2384d5b5e7143c50f200529df4648ed7515f29bd07633b7e681634ff5511b96d8ae131550f327ead6a73a737d6c72ff1d46e5cb4e
6b86a411843eed5a795572df6fe80d77ad740d11f1dcf3ef720d64b9720f95e0ee4c5be02ca19d862a1b13cbe1bb5264a73f67ab8d812bbef3f9eb26e22c5923
5834f4609d4fbde096207adaafee949587fb914b9e5595545731a4e8b561ab4be5930cf4ea40a9f94ea3f14390c7eb2a1678602e2c6fa1bc4dbcb09bb9e26ede
95dd42469fa2c20d8d5e465c9f199fe700489f39d5f7038f2bfd8f3b08514ae4b518bdb19926535aa98b3b4049bfda5364763de2340fb9b4ea5903744794642d
320fd16311f38129033665b0248bb572306695036fb5e35bcd67d3a3e34fb912af3c9e9e7d9d62fb50f3ce234cc352b6a6c37df88cbfcf84e338ff740312f05c
a4932fe69ff8a01d3ceacee11595fd49cf3fff51c8fcb9a1014bb0ac3dbbdb177792293a50a1edd80f0acca3f36afa59bd6f269819c723a82fd6b299da6dc8e5
05f6e8d16be4749dda26d89587c7346079efdd1658408851f012a92db2c47338f273aac7d643568ed81fb3adf784bfb901178c9b37ec0c8927965ead182ffdad
3582bdae015e40c69a23574daef485a962db5cc70072e6851cd842b530def376689fc4d6696d5d40987e7be20f4ac6cd82311a7fb108c9cb41585fae42ad483a
14f8bd988ad5af4e1641751f85ed3f1ebfef343b269558605d27f3a093cb3a402efe80a1ea4619a12c2c857c2065cc9439fd94b3b6fecf8d2db5dd21ae74f29e
d2f94497d91213dd3e8b8203e55c12d9aee8a565283d00c305fecf9bc92440630606f47d629e4fd0354eff0e769c1a03ddf91fcff710da28df3fbed0596fbe77
ae49cdbf5692f4565f3df97610b8b5512ad3737f8ab18431ee33313536b59ef34ba436ed07fa9528bdb74d74dddeb0e4022aac5d1c1419dc9ab084ce6ebc6921
ac49629a500879d31b8b313e90ea77b994c73a6be34dd221d4f6e2bcc8fdefd544fe436ef15b3d33b9143b9b99f044d19965f03b21df9eacf44eee4a2dc7ca1e
8250932616f0350867e2ac1f0ad5d0ac823359458249d51a5019c3a4da8c31677fc381aed2f0d7083749de264b57de10e9501f88cb6915292c348d6aa8e87aa6
ea040f005dfc220e3bcbc503eb6e0708b62977fb18f606c38f5652424878608e0d1add83efb1718dea0c6a207d2765a9230fd0a873f7a1a72e3e3211ca241e19
d23119cc3d70eced3a0ea63302648bdfe5741149db944f9a9b04171db6662a6feaa9da53e1f7077e67fac6c50b02b0349e6b106695785a756058304c3a69e7bb
5d234466cbad71694a7184f62f25cbfe5abfff8c1676e7c69dc5b82429eb71eadeeddbfa8c05e782b29287332b16a6a898648099bcc0f4c776f7eea2a6dd6ee0
b4490fba7ddc9a0144acc6c2c6f3ae925351f0562c96792e1b05adc534df2c6469df596e8604e2c7afee129eb9c5d75dec53c1758926fd3b23ade5861e02ecac
6c10834aefe227e4f85503593c52084eb5616e13c9e101c67b7620d00a6273551d3aa66cd3763ba50ca0c6e728b126d9f3583ac4a5af2a0ec7070397d7214bf9
64c617ea48a1d907aa76177ca5e02f26146425672de9567d5799e68f2bcc5fedd4f0cd29458ce9e9cf2a4f64600bfcdcbcf92d13c56d8bfb1cd77f1b048e4875
032cc26d5d89c9d23b486c905f032dd9d55b546f53e6561568de35292c382a07ff301801360c8ffadabaa7bcc1ba30ec40387befd60a0b1ae7fc2bdf3c96414f
29e5e3f776523d05ac878f34f2a72accd77839232a63b4ac0916cdab473c0ea497fe4941a938f6f9c1a46715fc950cadfbcc2996c34a19e54ef946660b34cd51
3bf513ad4c7137cf95c2ec8eb03323f70f835cfd943a25c0cf2363550ea31465281239b5d07919e4fab63171649646b84d288c5ce28d46286b5d4adc072833d2
a2ba847803c7c3359f3b98315ac5bcf480444c81b7de5ebe98d7929da222725129f0d4ff3c060db71e5752b4b1dfc5b3a7399f9860c691ef1b0ea9b30b5cb03d
f624d3f0487fcb9bd583ee54bd0b636914cda156f83dff4acc433044b071ad0df99f2d74578387d6f3564734c1864d0621cd977adf1d1938b8bd7a15101a69f8
3d58fba93304abbb786c2343166e32a9f64146d8bbe3d2fd05849116cf25eccc8650560e897983571791484817630bba284d9a10b785803c86e7a86c4e7db4f2
f5e556f590955c6242d7ec3403cc030147284003a264fe59af4718301393d40707dc4b97c3f53739b2a3b2145da5d499b38fb82044a4cef23d842a45cbafdfda
47cdff7c2d727b060bf431bb49e4d7671434c0db132f504b42720d45f49fdae093411550cde897d582c46c17c52efacc51bd68332ba0795326096617821f11a2
90b9c6acb43ad340cf1954e227645ae4a9bd1c7624f30492f3ea59528bb225e8f8e43f1b9cd4ebab2de81170439c5c27ce2beee3253cd6ca2aaf6f38e14f46a1
c1bdf6160d8852428c29ed68512da956bdff5dbc0624aa04e4fa3d80e4e3166d5c6b660bb0993feddbbda35edb55dc9d932320d3f3e19d67ec36f3bedc0b7988
9d70d1004baa5d7c6c1d4c5ca5d7343c85a6220d0402c04eb577967c81824033e33498522fb6c3f0d2f26dcae5590513abf058360046b360a472488f62b9efdc
68568c3956fded8a3cd6bb2e518d9bc035ad5b726bb4a30a0a6ea72c966382ddaa661ff18d33e5ac70900e94c11a02f3d98abc2af1c9ef3b3071043ad7526b01
8131928ac854e34a4de37ff8af72892b7622f1606ec6f3a6a9e4347bce6c628d5934e3370e580feb832bd64cda1e8b31cd696c58fd31737311872387ccc378f6
56b99035f975a255ae40fda058250bf0f7f3109754339aefb8f8be0b9af3804d74bf07db626c38e58bacd8961a3deb930f4e4959f9290e16931f90db0184b942
846d2971fb7cdc38995da4684d69929e5cd34eabebdedde00d2497b491c4c0ee12183a39fa13164f474990f6320433763f0ec139cf578e3d4054756ff993fb8c
c3edd409e3c091e1b4ee805b6bc254bae489c2fcf584abc2105733ffa7735cd218312eb05a3ec4668fe9765d62036140bd2a5f66fbb42edc224933f71a0798cb
259279c13f801ad86a146a17f21b2d2f48131f7bdf970106c0f1cc4dd43e3bb6ad177207d1071214e591ab794cf32286fb7e9b978a31bdf82647963b417ee30f
188c9218cf0cce176c5d191b0a860add13c9ed85ce74088279ac18b88039a85ee0e0e1bf6d128a2e5888f475417315ea171880b17b32e3276abd2d940abadbf9
05b194036ab6c14c227d31fea8697a7b7ea37edb2b2bf7ed747d489aa32dd4f65e053d8f341d0524cdb4c793aa312beded9f45240becae58379fa1d9aca5416d
8a3b95398fe759837843650160f0b69de4e933edf82483b272a5a8e2a1d3a61074c768f4f5f946260d2f651d8c7bf5979ffcbf93739dea7d35ea3bc4ffd70986
d1a8adcedc818f7a19ed7563a400c2ab216b65c795bad670103cf1599077183cc48222d0043c69ad098e638fc9d27d8f97c0fcb3e70c7ebf39a42356f724cf71
f8d9a41ad086ffa7aef23d2c405356e31df5f4dc093d98ab5496b8fe6a45f0ad54f209663ea31e52b7b0689d9bc86243685834ca8f650762445f0a214edfa937
cccfb9a26584def83db50bebd66cb506eb609a3b80818fc18810fef5f31b04542d31e1a3860aa5b0cd1264825e186d5098b3ae2e232525a0506be2796491c958
4ad7686598e82fbbbe51c46db96a82686e8135f4eabc1a3b05dfaa35b95e50eedcf78bafc9173dd8c0559644a1294bec5e0465992576d274b0f5b3fc6fce73c4
a4ca9dc2bf0757bba0c274f6169d2ac5d499cf18d55c3e2b3c05a4ce0fcd19d83ebfab35dc240a86ed1e913feb1b8da90859bd0ea913b1c752344fd8be7e12ab
143c2a9dc0582d368971bf6d8581c5980d60d488ea3770a729edacf0aaabe4ef65c8c12fb26b99aee045bdd09f7c4c33c663955cdb6d8ab3ac7cfe9e55c90ae5
3cddd60742d0a75f2b0c65eb75f6a49206ee7e75e3a57e201dfc2d830ef19c1bced334835bda47f683a2ef78a9eae88144907bb9959fe94449c87069408bcf5b
4abcf5f5fc02a79674836bfe076b1c3a2821e778577e5188262491677fe31a26f3e2f636f90d3e6e08d8f5af9987a89e35f0aff552d93ea5637699af3964a56c
555afd878f9b49abae9dcd0c1db4e806689b0adca66e83275aff1cf78c131a8be93f42ad44962e7a89042abe65ae7eba688043ddcf11b0d1af9a222352e62d8e
e4a25aed0c90c007ad487ae6b4646953014b2aa76695d876b04e3c0e6c1eec66c3253563fbad662bf83e00cdc3bb92d972cf08a14750444f1169bd5d5545b350
f69cb8a00a0f14d1c5530b72d23fede409ff84ab3ff64744723c9377859ae84adb41c16504175ca0ffb6ce6e8e38aa58afe3a82bd0437e3394b9ed25c64706b4
1b75b53574d681c9cdfea490289621d44f57c5dc67b2784c346e48a0393f7e93e42a81c83ed5f9732b7fdd13888ad8b203f7f7d83fcc133337bdc1bee1cd0650
a39d0e8bbb78e1ae856acb5f2ab7a932caf5c68fbaeb55a036995c2622c84abc4c392a7e17480f7777e640970fed6d5cc8b8dbb7ab24a743fa37b7e81eefd250
e1bf42b2880fd4c6e99a7d4a01b322d567550f3a23ccdcf4a539b5d8f258c67b2ae38011b35000ac90b256f56b34b5e54e013993b553f64aece32a7b47f6c1b1
f32ac69a4e69dcdc2f0f3afc9f9116295a1019c6b1d5b163e57a26f601c63907a5c7bc14478d968d0ddc86f80c1e472335f25a58c999833c62f641ff3371c522
7dbe4e166a104a85be61cc492756ae0e27954a132d05ab14370c01c37120fc9ce7f8bef4eb7495c85b0d941ed7c4b6cddeeecebaac467666bedb96c4f3e9d166
a5cd5e06c584c64dae74fa1322001a2ab64692baefa5993545d3d5f8734e314cfcf50be5e0fba2b2549c436a92bf0d5b8680fa51798eda18e2a14009c6fcc18c
abdda4b86f982e2a3043725b2472200a24ea3314625220d7612ca6f6f5f91e502568463265a75c9d96d8b70895212fb1d2e02ab28aebfbf9fa6751bdeeaceee4
4cd04ac4cb0d545630cf48a60fc59876ffdd5f1b32af947f6df095a9d349ab4bf20ebd76a7af8aa64a777ff0a3c4a2fe82ec684b1f770abf48afa66391697cd0
9864900d2be4ec5ea38e86e954a031afb4ac0dac5a19a114f22443572bba2da2fc1dc426e538771017f117744aacc09f0d216167f0b2b5c64098bc568a6a8c39
e0ec6531c366969be178609065a6fcaec5ff97230010809ec531b66db28d0913948e630f50327e880f2462cc01fded501289f680867e01df719b2bef5bb4abd5
cbeb94fdbd284abee51a4a546c2d3130857af3abcc7bd37e4603f4297c96360009365228e3dbc09acb69df1f1fab08fc3dbf0c1f2f82273eca7f1a9e6d3040e2
5e8c183f5e9519ad3426ff70c5c1957af7e335440efbd0b3df18dd2228bfc74305256eb8f25d3176494c0b1038094a7f66fff635c24442f257db78d1d1ac06bc
e74fd579f2d0d2241b8c8ea9a9ffc4fa2fa288897aa76b96ad88d6496ba6e53976a5fb7cf8ea48283cc0268fe8d681df3fdca3453b11283843d61103a89d7778
b740a75c60528f337d77e55783079dce6732dee4cc086fdaa0fce6b42c5c66abc8797cf072e0cbbc0fded2d8b78b8f55761cb172521828e494d857d95272159f
d5706223853eb54570a5801c451f447b8e7e537512c409efccebc8d5f4162b93dcc9b0af606d4e49c7407e8fceeb6e4d17439b1becd29bf1f2ba6950348106b0
ecaeba9b13efd124cae84462c8c7119046d6d4d8602518f27e5f32eabf8aa48770df8a9fc19834777e43bdcda9275f2a1dcd4df0ab397ffc6cd7975a5ac23220
4df2410f5149bb2159a3121065cf3892942e853f791ca5feb99cd79a67e44121343e4429aa7fdd396f1b5c6b2118c5fecae4ada5058d2fa4147f865d10baba03
f664a8c6d1e9e19a0d2fde7a7e89178c80717e7c0d128d091f6937ca560d2a7f0348598d5d65b3992243072853ea3e3f82b2b5c6802254a56fb0acd3c656f27f
ecab611e60b0bad0ae24334ffa31a79d9131948c48ad2b2f99d009a5243b338aa356271892993c19d2a8142303f1fbc38572a5530f6457d086f71d6c69b4cd98
73d8f03c684537417712c46830630d3a6e443badc8e646cb67ff8d180de511e96d394985d0f2eb97e68fc85f7e407fc7438b8389c02379875f040ada0f6539d8
86cb3cf67a1abf7af7d02347d16ed23e4766b3180d38e62a732871f850f6da32780dbbe5030865571566649e1e74136b9d425fc05b15d325624d3d758ae56ed5
e8a8606c9ce234173fc4157471272a288a451df64724e2d2ae58a0100002a5226f3eb104bac3998524f9f24591acf1de011087cba2b84cc37d487ab1b95c11f1
d67deb05421813c830885ff76c62c87424c959d9507d4bd63982e1c9c5bb915c7ba59602fc9d89073b758a4e1a19197666b68a3c74323ee316726f64ca71d122
fac3ed0056003ad3407caff893d06cbb41292393977cfa04d6556427e8aad228194d1b0a7fcdd1334d01616bcc20c4e2a63568ab1d5d3ffd468913790e6a97f4
d167b313be01b29950e85cdc9e294bc74a390965635794dc1d0f4c3ed0e56977ab74090bd34da04cb4d82ccbd260fd346aa3868c4798105f700dcc213c6226a0
12c18292a1ed56ed41b7880fee9c51e4cd44b02ffc15280c82666e724ab56bc3e2e0653250c37d7a4efbc366e299815a14c913d8917e683cf34ef552052f5368
d5afe7d7735ad0a7e0fdc3ef1cf7c40b0c42557ba52cb0ae38c42524ace187d90541a17d8949a229bee1e2b57cecf650e8c22effc4df44a6f1a0c57b180bccae
103585ab885cec64b95748c763ac582a5af5ac55c77ab373cbb0f8ca0af987069323ff516a7812b6c92a9e3aa6365b6df37dea10843921b5ec3f14f08b47bc54
add1adee8954faa8adba954932da0401149b772743aa260aaf16d53999ccf4f7dee9d404b5b9626534624640921c027d8f56eb05fc2e55bfea8b8f4c41a42ec7
3c15e3f52e66e650a5546d53da297475ab76a98d3fef75c970a88f35ee055e251a7ef36fa4744034bc165118feeb6bb57c1160897c2a3e2b562045b0296f8616
0df26f234c1443653ede413233cc345131e4ae766196dca605e934953290c4a0a449c406bbc206b4f037e3486dbabc946082404f1d3b5f7f8151e2dd0d74493a
a8507917130bcfebd92bebb9d578f348ed031133160d73ec82f5d103be3403a1e261de371e7a219b8f96c53600d5bbedd8f0630a66c5e8dcd42b274f0118cdd3
9a0231efe381087280a72a51819fcf726ee3dc93dc346f91ff8775ad5693e0285d406e1e532d1508327be432e0902c89453eb7ebe051081dfaf256c29c225f85
d4ecf181b7382d3bd1f75f97acb3ec591d9cc78d08e942e04a3c45b7d84078c71d48cf4e2d02292637b56d376fd0a5718a3ca05b89f4833da16980dc2ff24810
ce4382e8d37a5cc8d7ce9c92aa8a133451fb7c1b32f2d17c6da7d438ab1cf0109498519cc8ddfd1ad8509adbfea0e9a760d47b8cc45da913cab710d5535495ed

__gff__
ff9c4a2c0325832b8c1003fcff4625be08e4d85d24351687fa7dbc1eb5b859437a8b811d89b9ea4623464e9f2eec6795f506c52fb012c718a61c7a487a790132cef92db5888b88d16b2f6e30a734e8cd419af4f38e48c712c858467e577343f0c56516543130575002cedc310fc30296bc29179a721f8d46a9da31060ec50b8a
ce2a01ddcd8220cb922790ea3c8f10b6dd694cc5ce07d3ff2b0bd5be2ec074024dbe5c0920b42ebaa3efe924f9c523482c4d8554fa2e18b843b9bf1ad4044bf0012c7ad81e5648f55a8012f558e724d2cb0f21b123b05e32a5d21b7e14611e2da00f829a974ed6db488f3d24f6926218a14bc752e96be87a4d1c8d1de6789695

__map__
a7c9c41aa1ee81cbeab2bc56501aa31f9037323ec304a9ec1550cb7a8250245eab105d12a3a65b8ea1e91f3e64e1bb07210c96906308b416d1922937a5bb7ba41bc211df00cd12cef817f846699b700cfec5aa065b02f6ca84c593e85154779a32011d1b70d870432cce7409aeaea525d95c177d504ebb528ee040e32b57056d
5352b3f0b3d33d6ba2aef53c11a7f62c4f58a91345f501569474418288f86432db6040416184744efb437b6142da1d8c601b7eeebc3742052d2f9c156308782cc54263df01bcbdce1c7c42cbb199891edc6967db09c5e645c4d329486d4aa8431f3d4792c876a749e5b1b777861722bfc0b8bec8faec8a6b7dbf8ccc2d68425a
9b00a92a5c4abcc9e5370e847b385c3918675d39a961ed7e73003fb5432be28a56b85de88f13f650da47eee4dcad66e03811b4e20ba1104f54295ae63d09736c2f51636cbb4c505f7b4bb67454e76509bf7b747dd2b1fc21e15cf356c130f34c3b752fd88cf6ae8446644503984b455bc1c5a6b1c7f72318cf8370fa793a4027
c5c19c7fbc2a8f1ba8252b2d822aad802c6aefb51f04085d502f34249267dceec1f625dff7254ec0eafdb769632ca960a57341532b0bfc89e9f0104dec99dfc9ee92d6cbd18a10d7ba7f61c8d5ee23b4c7a61be05bb355ed6d972a9acb9dd79a89798293c7788e978d805471ad4f6837530e4153f1fa9b69dbaf44170198d9c2
47104b3436eacbc6aaf0d2628b968acc7227b16956d541769e958d63097edc01a48c142fe92e86196a147b1558dca5ea5664470404ac366070768dee23deb0ed4fba7158b612578f570684149e202e55491b1ed8e3ffa8d3581cb7e4f4a254a1acc9847f76e25342213e17bd12e1f9174b38b5a3f3554fc877e550b9443c5ca8
c66066999e09fa08e050bcb2a6ef85cd65ad785effa55b40d2cb2440b54932ad614aac7d7e4d5951dc2a80a36637118752254352b93b4ddfc9033a81c99401c5fef7aacc9c93d469534d452650921958639f32fe4d83563017ddd05010b65a6c630a1a58770b5d511ecd5ed7756c66fd2a3c5ce37d930264567437b5e2fa0cff
c31db5038724eed3e3bd79bf504ccbcc5b34ae695aa78d0e940e8f4dd92ecf00e9f8b9c82693de79ea6c3c9089a05c0a5315c4e1cc910588b37061af97f83bd704eb01061bcc3b60bc9386e9de1f03f5d1036db317899841cf8adcb591c443abdf4ad55530a769e08f9fc99ab7da5377a0f249ab635107c4522f74ef5d87baaa
5595b4a3c3afb2e21a11b928ee98f7b6b49e93e3b9e68961a7f07636d22675d1f52b5ff609d5a2a53e9da6a7f133bdf698cf2fec5c2976c48cd8a10131af707a532d9959c89ebe5bfab431542ee2496e2e0a98a386df5c325606ad327f28a0053ee1ba4dd67b2a0e0a64c03372074f94ab37d11a3319a1496e6c2af6cc4e26bc
76b710aad2b9e72acc7f5bb33c97c147ff5e05dcd0a7952c882d213ea6a9d81b31604f6084fd6d43888e37a60d1dfc80558e8ab37ae4f3e860d33c816962f6feb776c9fe0837003b6d054c4b851825721bee4088201f83658b721f93448e0f4c8d655b2f94590865d331169cd3c61f04ce51411a7cd42b6d00aefa013f74a00e
6e261e55236c1f2cf45308a8d3f352f0e897d1479945b42c04a56824089ef5c065dba374fd2e9048bbb3c14333a25dc9d8fdd46ec1089e807e231f942d911c6c46a788d8e1537da8d5405256f40ba0660100c79ce73002383c002c474f6ee1c0a92df8c7110877d337e232a2b8a49406802b6a78dd0eac898cde413148042447
e904c41b2f8a4e7b6d957be0092534a33d25ab09c311e5718dd012474e202564d2b2408b0fdda789c1954ed79c032a70bf69f487bd2fd55a34e5ce0cdcbc4482ede89fe2516b01d39f9447a86919dc4342ad8b5b850c89233f624e70d797a9528d0cc80b331525b96825703c19d07f43c1165163e12281069f373f3ed13c4b18
39eab0326ea7a530f5e0f4e17e3d3d03f446c8ff36fa71ed30590bbe10589646c553c1ccbe8cf0ceb2d37b42a13d94c20f066b1859637499270093d20a7143b2cfbc9c77a79220a038fd84fd3af7aa342efa347bf3ab84c1e57c0499db8dee41b2e30e61e3bc8f7f163fa9705e929ae5c155a8cc4e591edd840ec64f26a6cfd4
b46df1d08c751ab780b04f6d2aed80a6ade1fefab85d207845f6c4e651554f332ebfd3f3d5195b398c71b53d065505d0385c8f21d02ee062c6e1a019665de588c73e58ed7071ca815ba3774e5f5253c7b19d14ddeca5e59b6f8b0e0d90dfbd0510351552b971881755b6765057b214bab137a9a17c96676597c5d400b7348825
1eaca41347fc62fe9b8520c5826ea345ce273fb7c4d8fa4c8c340d3e70058a4816532b7ab0a386433971efe48e4b844278a3509ff1dfc6675db3b15ae9d81ab65adf4c508cf45b96ab2c5555b035bc59e5d68502557829500dc5e14dc08a1bd652f910721277b65459e227c01d04ef1ac8c3d1fd4c96b00a7e568d2ed47366d3
0892d9463cb8936e5efcbfd1d689518d2faa3e02f65951597e9882f578bda6c8e05b9b7c4d18e56d8200a7aaf852c87a017f28d19fd397e886e1cc2ebd8de4c7c2b92b114ef5309a1ec097bdd1a177e8d760cfe60cf3455ddf820eda6a1cf6d695fc0aa09443e18d2ce0dc6512c50f6525f34da67e2e0d985c9506727a9039bd
e0c52ae60b858787142e3df0cc873a13c842a06e13f14d91e16012e5c45d599a00e4a5e822d0d3c35fcc4014073e15d68886ac58af02509495b6d0247eb7058ec17ce0497ca491bb7d82eb807c843bef7b8cd4647d1cced924b9f54185c55d40cdc2ed56bc476afcefe5bd0937851873a66244c0002991ec429719269306fa4f
41dac95490a4157a70fdd38c41bc402fb70edd3daf490979f835a726fac301fff652fb17d2514a26f66e1695998e908fd0ed47a20fda22f1b2f75b28bc6812b0bb23ae88cb73c1ff0190709a9380745206cc517bc80e04ca5a2c9d667ad4616e9801df1f15132972264cc33a8a1a5c7533c19e540a7f0ff8b89f1462e29c2a78
dfcd4d57ac0b704c2904bfe06cc1a1c71cc7d0b142d803061351704b408a56ebeb735c11d14ebf9a8841a999a72303cbb699acc55b43ff1adc07d3d49d0e09e4dc046b2bb02b602e72df3ad1be1b4589267a9e81c8b63ae1ae32cd9c6dac376af8ec0be157ff28703550bdf7664f646c98154a75cec585ef757a4147503802d6
7589469f240faa2a8a4c1ead267300c97645d44feee662d1cc9f0e06b17c1aa594921f914f80dfb5ded4b0d5017c6bc8d29e8af636f828d1aa0964dd964478c343523a9a780025ed5469f955acef2446ef1a92c1c268bc9e8c0ade8c670e46c3114d2cc403dabaed8afcaabfb358fd8001cead491b2dcb8a3458982f90466dd0
7af12e07d619846d151a17082680d2fff48a4836bc809a1e6bcf90af780a14d05884172b4a80e694173eef9debb392d85604ac83d5775d0bc85edf963e60cc562b22e494cba9d0d474e47f3e0457f11da8e18eea82edfaa621e6e2b2af19bc5c68ed6040fc247c3530a234c340f0cc131e47f44c778d9e17a1dfaed76abe0fe7
3563aaa1ae1b42f788819b632929f1347a16ce3ac21eee14798bd98df907dd902d64af6f63bfbe54e73c6142ab9577592cb2b87dc0e699cf8c53bb92f83d2e0a05523c293798fc186fe9044c97604db0c536dcee3cddd204ac785144478126ed7dfb03fe2a498c3890567b2e9cbd94c13c7f4c62bcab55fec829106a05aa2689
8cf45709f7728b8c2bf964b238eea624090a97efdd361b113f72962a2287dfd530f2e31cad94ae1d04d672b8560c115615daa0a45933940cdf13a28a403cd764444811a74551bc403123689673675b5c36578ad9a4695c99ed058dd6851b7def78b5f7bc2a5ccba28fc5b71dd938fb77cb59e8cda7a6d4a7496cf493af25ed67
a869dad70d3c49f72d3b723c6e2070f6f4c1f5038df04c863439c69ec5a845c9bc663e4906c8235dc9e2de0d76a7313c35b03c7812677886fdf683918212df612823c092019514dd3c121794368a98416a62d1f3f36c12af32f36439049e0529234e9a4f26dfbfc9d62721f93c395743b91272a6a1dd5c2d8ba5a5f9204b9f2e
8bdc9710b3adebc6bb945e2e4cc4bcb08c0f01298370cd581e300dd183f440fd5633b54744554908fa46b9108218e37b337b0a6ae4540d93bf1e704020939a792a4389fde9415270c7bf07e3437e87c0aaabfb90850dd19b4d0c21cade77e8be12ca4ed27d2914b7086fc5c989ef5a0957485690182e493aa7c3aaae90c5fc05
85be17674a9b572a4c71804dc52e53f9544092de53b75cc6f0c74b1637519b0541d313776b7ff210105ad3c45ac8f2bb90e4709b6472cee29ac9e97e039b632efddda1b5fd24052af1d2144723cfe0cfd6a7446f3b424c9bd7f2294960107ea7472648dbcc64f83f6fda2a6413380cb5dc95de7b24f58ab77fbcacfd8e9d5589
b309d4a0d4daca341d4edf57a0e68842439db05469fe09bfe4b391e067429e476e418a2cfce0c6d74d60324401984c8666dec05cf26c0a89d40bd813648b26b1023a9c3fb1161982191872bcca26a6386f9eab16103cac9e3fe81214e3c6f3d23bcdd32fceb665d2f1a0a6b68b2bdae4b7b77595cc43efa12a8ddf60604a65e5
fcd78a2b15a4b9d370e60f40e75fffe3394a4fc6010e2421484699f61bfef3d3f3cbdbcced91c769800b4594cb6e65422870e0b9f7d13af32a1312c4d0014cb68eae67a96bf2539cf28c7dc974062f4ddb449ca85f4901140c129c0a20a864f3c6167ad6c602111172e309157a82a517187edd27dc50244d2b9e8e0b7221b670
df6e943394d7fdb03cfdf3ffc46d7db02c5ec555ddde06ec2ca550e97a34aaffe4dafc906b0a0293e3caf624bcb2e239d11b105885efbe170460334a02c1f850cb45576d630c510f60485cd48eb7f90af289cbdf81cf6b98b78a28cddb68697bd5df6dc9a740efbf0975d6897da1b09a1b158ce501e68561ee7f4a8b0702d4ab
3b1409a05396a724e877fa623347b085cb5e84dc5b31bf76bb9a28d127a32f9bba7e1d4f53de67e8c342afbd5d835d083ec501d72d707a2df19a1a44de73568719b29414b7c9b14573784fbbece523a909862697db7215ff33a0d43e4ca2e653d421f74537c2ae6edf30b4ef0e9b2b6188a10504c7f169b15acec1718068deb9
247d31274e521a3778c139540375b75162c3d7d072f7c0e9fdf5110b42831f0e3d78b0a356e0b23289b8bdf4cbac35edf27ee3d4d7bc34cce70bbf6e36f915469c299cc5fd660932ed3953b5973cd6d8f08caccf06c70087f7c72b5435f1ca782181c432e5458f36023ec1fae8f9e1a543f338dadb330d2f6ad73d635ebbe5d3
d9d6f2261c51360f2dfb653c76d372fa811917f5b9ba7ad734f8644aa3e825a28fd5356c13c78b8368a73ac6170fca1d9be48153ef6a2dc2266f6de0f43f790594558371d757da903399859840587b09edb23d2ad80939fb626e1a949a215c2206aca855157fccdd55b77c2822607948490cd568d6697786fb3cd81db95975c4
aa0e86a0b5b8f07dd9a6a3fca18e3601a84e689ccdde8e0d3e0d128e22068e3acfbdda0a2d03cf985696bda08a0add7fa91485d2f5ae3914a17afd5c92d797d1c6ab702024eca2609b4febcf16cad416d689968cc7732467bed2f1e73e98beed5d4c7cea1ff8fb8979a4b6ee807fcedf90840fdeb29eb6922af66111181dbcb4

__sfx__
798a7adb7a564dd1018e022179bd22ca223a6e8cd299fed2df08edcc40b4bcab028577055fd90a8f7d54281b262cc19f9e7628ea58e99406a893057f7b2dd9ff93626b2604bb68da2b6b53faa1925cab3ceb5663
79211628c6845953be71352a4f69229c2672abb612a7161990ee231db9aaedd79a93534d8a41d9230d3cc20145843c4d442fbc5cc4068f200b74bf44e165401e8af486e34b29a51a04c0c1905580db9f8eca1082
adc8613cabd47fecbd726c1d440236c08f8e20606a38a7599238e1b79e47b2df61a74cf08cdac45b55503647b61dad9dfdf0c8a7f88afa427b62faba5f8e8229c5dea33fd8a78dba2bd93ced7016b513c612af45
8448695fcd18d1218066902a7e7bccf3ba88d0b74c1df8c4f41a6c9234bde4b152ba670feaa425c5d86bcf89c0e0fa232efbb2489395a63d857af8eedbca1f0ba98697dff3ad54081fc793b6bee31133a8ee7987
28f45bbeac837a8f93cabe6a474bbb0bf9fb3dc14836d5769e090e38ce38ff793baa16f9bc13e06c302eb013e1ce81fd5aa3c43b7dec7b6df7bbd1f1e4468267b3602861e95f97dc7afde280601e7bdb4c05a5d7
f824cd324465285a5c23f4a9cae88cb65ba89233a26ce60a41636ed938f28c45028767a81335e56c060fc9ef10dfd37ec4b0fdf2d53f419c193c7175c9ac8de932c1a70c81f0794a3e132b36c4ce0e853a727c4a
5c0453d0eb80476b79620dbe7a2f9a8c6b30529124762abf07ae3ff2f061bde19248c0a2c87a67ec83eceed10c879ae72f717758deaf8a309819e632bc0a95f1eb9fd3331ffc9eb9bdae4e45c69a80608cbd0dc5
d264df1eeeebe16d4c7fe5e221e7b34d551d5965767146788c0c0c870cb46aeca4924a7f11c88a9fa733085bb940a7ed94ba0fdec051745372f12acd64ff5f62859bd53fc993d07650f00a215f9a2404502fb9b8
2c8f4218cd4ea03b5ce88517c1fa9660bf476c0d128ae474cf28a3e03b13dff404b24134b4774e086fa53aba32d6e2950eae93959e7d5c4b27259fe7def589696a9ef5cbf3ac97151780949fd101902dbf7fd495
cc44b8417d712813a4e733e55c928fad0028476a8e9557addff2b7055d59e7ca0164011d9ed2994ef7aa819573cf41deb87697c08a65b5f7ae673770f78a76157d93549f80b6e5177b1a2c1474a30f2bbaf18184
49ac9225bb24a71f227b0b7982f3afabf3cd7548d70d3d42bd7a400ed2a1941acbdee2966d0113129d3164f3618c3c73dfb2bd1b1cac5cf510f4f5039e46e6bd90e7071ceb38b96de28cf81dcdeb2af32d9bd1bb
c69721e64014bbda67fff7063cff35d3b9f4034814b9aa01a9116fce7f49c3c606977100297a5970f83eda8dc4f6b3c54a4eac1054714791bcee35b2b18350a2dcd13564f98f1c6d2a4fca9994045848c627e7e2
9b172314cc3837d0c208e2334ac49fd6921782826233390a275a3e099e1bd081ce39e620638bc8af44eaf81e8076e88e07737c925c920c075deb721bfc72120ca9d35a7c1c9018e9ce9c882e07c301ee02ef7fa9
661e332e3d1180528992bade87779dab573f067b70195ccb6c3fe73e69d48f46bca74fce2827863fce1d0bb3f7ae433c4a0c6b8c9c6e039abf07784f3cc4ba41a05c1012312ad2d2e688242701469c1cf4662188
3fe7dbad2442440d62c93fc27046d83d7d6a2ded80747275a44ce35c7e8a55a5d88987dd3329771d800aff2aab1c5ff15b3408c7e53d5cc0646a9205abfe7e6bfab248e8222f6ce06d4bced335b9566001bc7145
84e7531f625e7db9b450f402e094f82d5fd6ca25aba200b2139844d56106d0e447a147068486ea63d403dc73ddcf907608b8820c3dc8e0099a03a3fd94d504933b0a0dd363a5ab57541a6ee97b819fc5a6b1323a
5aad5ee5c172e80708b0f3d29457ba9b51abdeaef236e709aed4e8a32c0be13b55366cc1068ed2470f8c337f4e3ac176fb4bc387ec7100f7f998ae89831ec5d4ca979eb3c087a6a1edbae7fad2cf9691f61e3eec
36a00f6c5466a9076da4a15724ca65e7aa39e48164478ff29da17babbe1411d3f826d98b8fc353724f0da0ec345470bc99db708bbaaa14d6ad0fff9674fd618d1057cc4d05541eba0c3e1de02a351166891edf53
c5267725c430a70f255d11447ee85e1a9afe541c645025d198ddba48fd0d0d8e48a5be6ee8ff80e08dafee1d24e494c144c2c83e00a6f2d1acc062cc9fb24e63584b7b82c02280e03f0cf3b51fe31f91977dbece
49d21a22101f13fb050b14b9f5751205fbf8ea7dae985b4c85192c93b6e91b267e7edcec3c0aaf2e91b9c288116ac591be88a0d658b7a4e33321b28dda7527bd361e680144d84bf40aa95f3429f88e9c0957e3f8
f528d2e5fa18b45dedeefa98a8e92780346d0ad5ea268c7034899f7dbd32bdc4471e0da80a73c144e97716e5f2d599ebd6234260f5f1cd2a9bacc7bda92055a425cd1a8c9559e1a417275e256506dcd94efc4da5
f4ec0740fe78e20d5778bbf7ad564ef118a6abf614e745be095814aaa9d7ee1933df2bd9ce7cac0988df7655259abee00d578a85d81fd0a6124177037f2c477bd1a2103b66e86ace04ef5e2acbdbe9bfaabb0aa7
c42e39b927b5a4b41e52ca32529bf99a41614fc8e604916b752599056d79082521b5bfa109f82474a9d02236978bf266d5edd25f79433405262712991512cd9b563314266ac63e47dcf192f20623f91a03765078
8bd0f28fcb1feb4be472c63c06f221ae06c61dce00b8ccc7476e10f3a00f281d6756960caf99657ea0f54a4ba8ba363d06957df6f6c6470187d8eb9dd8df83c7f241c73eab5efc138b9033882be5396c22dc339c
06c225c0aaa8cb85c7c87fe280698a8abdd007022d44f47cf843e6c7b1ae06802db1326d8078886fe10d5faa395b89cda942fbd4fa5003d2e689e9a57a3d675d8f2112e0e349aec2718992957ab466f5fbcf3614
dac8ef32a42e1820f33d329c88c2371eb959f6a57ff9fe01f34a75a352055cf1dcd04db66be2d9c43006f00e17f1dd751f17884df9ed5db1f98acae4dfcb8253e8ffb0169a4415d5c7164c8b5be7701e81c7a2da
448ecfb34a6f01b6cd664fd76844bc57e47f55a63177f10946816e086676368bc24bf08e7e3b3fdc0c9bb9943bf7e5d90a64e6db684617ee662b0753bdbb204a0cddc1ce79fdf29d74b207a594fc543abc7e96d7
8a906ba9dbdb62f1f6114204e6cc75bce65569e862dc033109dc0e64d99e6fbb82fd106f3d9b309bf78dd98f67e1a40acdb26e9ee27e878b1ca61c5a1471c8c58797b8ca5353bc17e2016a9d9cd0916fc8e8ed32
d2553a17cabd21193a766b4dc037aa005af6030259be32b1c2936cb1ba337de2df2f009465b0f3d24764268b5418116f2ffbbc4e80cc325d4171cb2daf2533d1223477615abf41d4739ad30c3c19b0b522416d03
c83088d12f38178da88b0c0a1d4e8acd7b1f84829409beed5a69ecc2b613c9f4c3f9ca04c4ec23cc4cd06948cdf13cbc5ff9152bf4624a4856a4ac10a9c2e53b1a40b7a62638bd0a794744302488738280f786ef
491433d4726f2301c495dbed6abfe9b4d6946f4a9954a6a32e50a178ab75f508ecfdcc14f9ae241fa626cb63b4c431f92887030c289b4c9be98662f33b7778e5de020169799fbbd22fc143b4f86ed74f91663555
7a81e09e6396ce9eff80e07ccdbc00ec45435436d5f8e4d866609bd51ccc18b7356886a8239db348530c7e2f097c832dabbb8527f636d6caf5fcedfffb6bd2692f82b7095df4660fc3b0132273a909956d8f5280
20c3c8e092af70cece7ce51c3156e5d9751bce58bf3bbe65fe033a93ef65d0dadd21af6bcf966d88f15c64fcfdbc8aecc88e2bb78e8a1480b7ef508ca78a07eb81f0b766bc6537dc9180f7e6182b0c6580a314db
7ed669ce8d8cd1764f6f83a41921205fd1f3a3c98eff6670d2886aeb8f6a0b5d15e0cfadf1f23557c36346fe8b9367fde0a0f21cb30d81f1e69e86078be6da90984f51063e7c872dfa8881a60651ebd0a4d13b56
14cc67057bcb8070e3b815e65034b62a00946241c4193171b99e6befe648aa73b25f464d84b3a5bbed2aa1713da98ec3a0993a0489d917a0edbbb6ba2144eea9eeff8b5b9346e497ff10f19ab99de83dd46adf34
ab7a05ede80c9344f4fcb629b3683f11a9ba79bbabd53d66bfdd1b0920d1962f2bafa52e77cc172d1b021bb6223b6515c911676d7f1507e52699e3dedc7840792595cd9f84d9808e65fdd7ea2fe9859ce298c6a7
2aadc946804c0c8ccd23544748cf4517e67f81c26df9af718aacaecbec01ba24cfd8886d75e1bd38db688511942de8e3b417a22b0adb377aa128655fdd65f1846630c12f11b4428be9db0a4f05bd3396a6190ffb
371d20443ce9a6e39e7419724e9725dddbc1576c2dba0d48eaaa4de417cf14ea300bdf1d0ae22522ce657b6770b89a2daf1e964371485c8daa3041922a91a31769cb77d670afb96e1093c87ee143e08e135a576f
2ebc3eceb1b3f77021bb2c672f6c9f0891cb9b4a6be0fcb08c50fad45e35d04c23d0307d02802e1164fc50beeff128e4c433adc9d0fe6abc3541b36bc7e95b11fb3de8df6b343ff8e9fe7ba967bcf35b41261c8d
34cf32d66b71627f61ac4fb77aa5d5148f8f3355c69b3e754472eeda078eb1aafff2e9624969a6b722afb14e51816cbebd58663dce0aa0c8788312094ff86b843032d3647bea69232af169ebc12d95c59aa9d505
37a8c54790f2e7978cc1a8f11d1721eb3c6a4282505593fac2c1ad82a83666dcb04a2ada3e3bf51a59552339583939026bc6395da403503e914199dccb924e537cea3d983a64303361a463bc579580448c5cf275
04a7857de1efc0946fdcf91ff421ea5910761b20f49e4b85db92d65fd7a57356c65ed0f1857be72b722d526a79e1b0f0ffb099c6becdd4270ab288b855baa3661a2b93d6b330df9a37d7cb0a082f7f44784dfe16
56342e27b9ea4325ead7f62296730c571da8007307d9b21142492802e148d2dfe6529f0640869f86699431db2109c02093637abae941f447bde42fc2fefdfeb26fe24627e518f718b38aad2e7830fc9173bfaa63
a7c41b89675e2a80d7ade041e2b6e851bbba7a891edee54311e40809f2359c3ebf08309ddf2d884d499f18df2303ee0775ae19970e749cdf8161873ee48c59e0933612d974728bcb335eb2f10787e864bcf90a6c
4a24632a8323040c0f37c611b452985de7b684ad33d3a984d28aa6ea4c4b138915646bb7b16a50a0fa1f2e6b894b58ffff898c8719d911d0c4c9d4ebb6fe7b9c0343080e5b8b6d12d1748e0e4c4b3f963c164b17
97884eb47d2f8d8a9bbe35e63f94e374480566c4fc3ed80818cab325c7230b04451f9ab54edff8566a867e64e6d52b3104c0fe52868cca6f2bbf730b5062e70b1f67ee734f6829a15eb04d001f450f2538b57ad1
4762aeb3ebe065601d092ea0583701edfbfaaa0a39a19e57cd9c4e5da33941978a18c43fc869693dce81edbae8221cde4d92e7ea6ef125863d8b487e08e091f6cb2ebc5d4175f6af34bcf524700aeb3b7495acc0
00af6d1606827ca64ad8ee0f1d10f8c3877e1182f996ce2c7ed52bff90a8bdd349f9285fb7283f5d6852546753a00e3640988bdef2b83ecc09ad723c9becfecbfdcc8fc1b40a4d9aa52fb19f4f4eb98450d2b1d5
e7d811cf49dd6175287162677023380cb8aafab49b4c3a286de8be6a2e6e27abcdd76bd82e4ac133d48d9fd56c94db3fe6df1204f21a406e5343b218bc598d5413cadd676b14964c4a728071faa928514996b1f8
b06751e0488096200ef357a74ff7fdba8ec2e4ebf7aa939e35cdd3a76485e142b1368771d5fa59ef662417e334eb7bae29522d551ca210886c86c89865d284501e45ec7a5e1eaf3166b610111d64adfea9cbd3f5
9e0798d6759cbd92a9d6a3eaf6a2b3fccbbdad0177ea45c88420554a28abfb6e47fcd18294d9615f127f454306b49645b129c480eda61e69fce01e499cdcc97d096d5b0aa5ad5f5cc464ae5f79d5f18e56788ddc
31b39c21cc5cf588807b2dad27981773703c68fd246ff7025502c63b500473b46267c6290525f1573df5135b073efac00032eca6599786f3b2a9f9d33e1e04f451811706935653c00b658274b5ec5b4152733deb
9565cf032ce33ded405a3a6c85de7055192fa6090b62821a0eaa08a59dfe17f8def1070ec891a7aa02f5e5ee74503740b3252680d6c021b0591ca22f8f22e1c8bedc62581e0992731338adfceed2a48c03f143ed
9a2fe144608b79e5ee4a641b58f7bf5e566af05807cd69c9984e77acb472dec54f7a7790c513dd67813842bc68a5d6c5d566be1988bb396894223ea889af952959c73b2533d8f58c95e58e6a6fb3b701ea38b278
f571ae58a620248c97fc596128d0ddc30cd7e9342f1ca0d28e2b0335a0462d10815f5d74351c2894b1ecce85112928848c0f7f57d2bb5a8eafd6f570aafff6ced0f931b683a08717ad04bdcd0d6b9ec0fa252129
4c54c993048b91b7acb7a587fea1edbae7f8b4a255a9de12fa55f58da56cf81417a81e0182a2494dafa7a26fc7a59d294d5081b40c768d22ff6e45cb661932f2c17671b99297dd150627f15217dfdfeee7daec9b
b9a1683a86b3314c6ab671467626b29aa0e04c80d4d0ce93a399fd53d9784ee71591daf5b5157b6ee35b5829b2ca28ac99d8a470082b594cfba8f02ffb139ce17df4a140192ef1f78d173d3790f42c8bfa5fb956
4cfa2b1dc2bbe3abdd2dd917d7c8d0aba3d3276ad6548b24bcc2424de9cf9fd62d71b5f186c55b2d847034f9c03fc632d7e0bb7c0d4560d6bdf6bfebc9592090165f606495e9c6a47ad234070dad42a623352ef0
6cdcc7c9f5275da8a066f15807fb6b63648a4eade3a171bce1e24ca4992b57502a38eefd827c2dbedb454c3a80498968aefdc27daa012bbce646b6c03924e0dc3a7d1f1e4040763250dcfd7e506e7987cbdf0e30
bcaf96dff1a8ce672c80a8df71bd315a6345dd4a8f11a19029c3ae010211e6a8ce79f97c718363acf274360b9485b23216b9095925ae1747edecd1c5a26906f6e996036d756e05cc750f3b92a37732fd1aeecef2
441714eae81b5706d0ceff358f5caae26c9df8b39a7eb5be994a4d13b333adf77b2aed9df1b1d066278da15fad179bc676479b48ae1a2aaf4c606584e2c3c5ce8fd56f06b8fe5d36345825740c5b783b8ab825a7
28017bc70654df6a909ecbe3dd4264b1d5dac969ec15605be59ce6c62c6709c13f5c4076c919f922d78717313cd7cd548ba9290aec45b2309d21cd115c09d03e8a63c49820dcf9af742db158466b880e220e5b05
70b0dde38a7763f28eab8255955052b51b43ec035dd5a1a1f7482797175b15f8c4779917fcb8684b0670d839d741a0a18f3a0a346ef5311f898f732f3f80068bc8caf6f98ccc0e47cc19f8340306d64b490967b3
c04cafeb4dd9163d6cf2ea4827a7fc857706b7b4c1b65e790228f9ea8f6ee181de29bcafe7c31680af6c31093fad861f53ee2d3cae57d792318aede34caf52435fb582c093bab6e7186ed63abcf0f3d883890018

__music__
1e bd14731d
91 ef5d55a9
4e 688f4b3f
6d da8ebe7f
21 a64b1dfe
3a 6c03f970
81 5bc11936
3d cfeb4f0e
34 ee3ed6a0
d9 5e50963f
de c45e14b9
72 c72a9d8f
4a d19ce516
d0 c3ab15c2
78 0f72441d
ce dca90a75
85 3d87a75d
2e 0b271ae7
53 7cec9bab
4d 3a30e0bb
f0 b5b1cf89
24 6ad5ac4c
7a d01c5ab2
b2 feb26f8a
62 5e688a02
fd 5368d2c8
c2 a4da7444
8e 1303835e
28 187e934c
3f 7000a0a1
38 06690a13
1a c7ad1537
23 dd9d0425
9d ed8434b6
c5 1e88a25c
04 07728dbd
bf 3dcbf360
9b 197c93c8
b1 c4a64470
f4 8fc835d3
6b bf8ac30a
67 917b884f
b3 50dcbc65
bd 4ed547d7
25 fa7836ba
f1 5402ef20
d4 9b04c98e
7b 68397206
57 3dde1ddf
8f f4132190
78 7e5c02ef
fb 76944a67
44 84bffe5e
e8 bea153b5
23 99d17c67
cf 9509ea7b
9e f1c75a74
14 a9d2df9d
ee 2d6d27a9
a4 23ae57a1
28 e6ffd4c6
cb 9fe2f440
13 00385dd1
c5 a2afaa38

