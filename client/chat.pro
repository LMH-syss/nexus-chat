QT += widgets gui network

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    addfriendsearchdlg.cpp \
    applyfrienditem.cpp \
    applyfriendpage.cpp \
    BubbleFrame.cpp \
    ChatItemBase.cpp \
    TextBubble.cpp \
    chatpage.cpp \
    chatdialog.cpp \
    chatuserwid.cpp \
    chatview.cpp \
    clickedbtn.cpp \
    clickedlabel.cpp \
    contactuseritem.cpp \
    global.cpp \
    httpmgr.cpp \
    logindialog.cpp \
    main.cpp \
    mainwindow.cpp \
    registerdialog.cpp \
    resetdialog.cpp \
    statewidget.cpp \
    tcpmgr.cpp \
    timerbtn.cpp \
    usermgr.cpp

HEADERS += \
    addfriendsearchdlg.h \
    applyfrienditem.h \
    applyfriendpage.h \
    BubbleFrame.h \
    ChatItemBase.h \
    TextBubble.h \
    chatpage.h \
    chatdialog.h \
    chatuserwid.h \
    chatview.h \
    clickedbtn.h \
    clickedlabel.h \
    contactuseritem.h \
    global.h \
    httpmgr.h \
    logindialog.h \
    mainwindow.h \
    registerdialog.h \
    resetdialog.h \
    singleton.h \
    statewidget.h \
    tcpmgr.h \
    timerbtn.h \
    usermgr.h

FORMS += \
    addfriendsearchdlg.ui \
    applyfrienditem.ui \
    applyfriendpage.ui \
    chatpage.ui \
    chatdialog.ui \
    chatuserwid.ui \
    contactuseritem.ui \
    logindialog.ui \
    mainwindow.ui \
    registerdialog.ui \
    resetdialog.ui

RC_ICONS = icon.ico
DESTDIR = ./bin

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    rc.qrc

DISTFILES += \
    config.ini

win32:CONFIG(release, debug | release)
{
    #閹稿洤鐣剧憰浣瑰鐠愭繄娈戦弬鍥︽閻╊喖缍嶆稉鍝勪紣缁嬪娲拌ぐ鏇氱瑓release閻╊喖缍嶆稉瀣畱閹碘偓閺堝—ll閵嗕勾ib閺傚洣娆㈤敍灞肩伐婵″倸浼愮粙瀣窗瑜版洖婀狣:\QT\Test
    #PWD鐏忓彉璐烡:/QT/Test閿涘瓕llFile = D:/QT/Test/release/*.dll
    TargetConfig = $${PWD}/config.ini
    #鐏忓棜绶崗銉ф窗瑜版洑鑵戦惃?/"閺囨寧宕叉稉?\"
    TargetConfig = $$replace(TargetConfig, /, \\)
    #鐏忓棜绶崙铏规窗瑜版洑鑵戦惃?/"閺囨寧宕叉稉?\"
    OutputDir =  $${OUT_PWD}/$${DESTDIR}
    OutputDir = $$replace(OutputDir, /, \\)
    //閹笛嗩攽copy閸涙垝鎶?
    QMAKE_POST_LINK += copy /Y \"$$TargetConfig\" \"$$OutputDir\"
}
