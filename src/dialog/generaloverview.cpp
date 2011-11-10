#include "generaloverview.h"
#include "engine.h"

#include "OSCS.h"
#include <QFile>

#ifdef OSCS
#include "ui_generaloverviewoe.h"
#else
#include "ui_generaloverview.h"
#endif

#include <QMessageBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QCommandLinkButton>
#include <QClipboard>


GeneralOverview::GeneralOverview(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GeneralOverview)
{
    ui->setupUi(this);

    button_layout = new QVBoxLayout;

    QGroupBox *group_box = new QGroupBox;
    group_box->setTitle(tr("Effects"));
    group_box->setLayout(button_layout);
    ui->scrollArea->setWidget(group_box);
}

void GeneralOverview::fillGenerals(const QList<const General *> &generals){
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(generals.length());
    ui->tableWidget->setIconSize(QSize(20,20));
    QIcon lord_icon("image/system/roles/lord.png");

    int i;
    for(i=0; i<generals.length(); i++){
        const General *general = generals[i];

        QString name, kingdom, gender, max_hp, package;
#ifdef OSCS
        QString number = Sanguosha->translate("!" + general->objectName());
#endif

        name = Sanguosha->translate(general->objectName());

        kingdom = Sanguosha->translate(general->getKingdom());
        gender = general->isMale() ? tr("Male") : tr("Female");
        max_hp = QString::number(general->getMaxHp());
        package = Sanguosha->translate(general->getPackage());

#ifdef OSCS
        QTableWidgetItem *number_item = new QTableWidgetItem(number);
        number_item->setTextAlignment(Qt::AlignCenter);
#endif

        QTableWidgetItem *name_item = new QTableWidgetItem(name);
        name_item->setTextAlignment(Qt::AlignCenter);
        name_item->setData(Qt::UserRole, general->objectName());

#ifndef OSCS
        if(general->isLord()){
            name_item->setIcon(lord_icon);
            name_item->setTextAlignment(Qt::AlignCenter);
        }

        if(general->isHidden())
            name_item->setBackgroundColor(Qt::gray);
#endif

        QTableWidgetItem *kingdom_item = new QTableWidgetItem(kingdom);
        kingdom_item->setTextAlignment(Qt::AlignCenter);

        QTableWidgetItem *gender_item = new QTableWidgetItem(gender);
        gender_item->setTextAlignment(Qt::AlignCenter);

        QTableWidgetItem *max_hp_item = new QTableWidgetItem(max_hp);
        max_hp_item->setTextAlignment(Qt::AlignCenter);

        QTableWidgetItem *package_item = new QTableWidgetItem(package);
        package_item->setTextAlignment(Qt::AlignCenter);

#ifdef OSCS
        QColor qkColor;
        if(general->getKingdom() == "wei"){
            qkColor = Qt::darkBlue;
        }else if(general->getKingdom() == "shu"){
            qkColor = Qt::darkRed;
        }else if(general->getKingdom() == "wu"){
            qkColor = Qt::darkGreen;
        }else if(general->getKingdom() == "qun"){
            qkColor = Qt::darkGray;
        }else if(general->getKingdom() == "god"){
            qkColor = Qt::darkYellow;
        }else{
            qkColor = Qt::cyan;
        }

        name_item->setBackgroundColor(qkColor);
        kingdom_item->setBackgroundColor(qkColor);
        gender_item->setBackgroundColor(qkColor);
        max_hp_item->setBackgroundColor(qkColor);
        package_item->setBackgroundColor(qkColor);
        number_item->setBackgroundColor(qkColor);

        QColor qColort = Qt::white;
        if(general->getKingdom() == "tan") qColort = Qt::black;
        name_item->setTextColor(qColort);
        kingdom_item->setTextColor(qColort);
        gender_item->setTextColor(qColort);
        max_hp_item->setTextColor(qColort);
        package_item->setTextColor(qColort);
        number_item->setTextColor(qColort);

        if(general->objectName() == "xushu"){//xushu
            gender_item->setBackgroundColor(Qt::darkBlue);
            max_hp_item->setBackgroundColor(Qt::darkBlue);
            package_item->setBackgroundColor(Qt::darkBlue);
        }

        if(general->isFemale()){
            name_item->setBackgroundColor(Qt::magenta);
            name_item->setTextColor(Qt::white);
        }

        if(general->isLord()){
            name_item->setIcon(lord_icon);
            name_item->setTextAlignment(Qt::AlignCenter);
            name_item->setBackgroundColor(Qt::yellow);
            name_item->setTextColor(Qt::black);
        }

        if(general->isHidden()){
            name_item->setBackgroundColor(Qt::black);
            name_item->setTextColor(Qt::white);
        }
#endif

        ui->tableWidget->setItem(i, 0, name_item);
        ui->tableWidget->setItem(i, 1, kingdom_item);
        ui->tableWidget->setItem(i, 2, gender_item);
        ui->tableWidget->setItem(i, 3, max_hp_item);
        ui->tableWidget->setItem(i, 4, package_item);
#ifdef OSCS
        ui->tableWidget->setItem(i, 5, number_item);
#endif
    }

#ifdef OSCS
    ui->tableWidget->setColumnWidth(0, 110);
#else
    ui->tableWidget->setColumnWidth(0, 80);
#endif
    ui->tableWidget->setColumnWidth(1, 50);
    ui->tableWidget->setColumnWidth(2, 50);
    ui->tableWidget->setColumnWidth(3, 60);
#ifdef OSCS
    ui->tableWidget->setColumnWidth(4, 70);
    ui->tableWidget->setColumnWidth(5, 50);
#endif

    ui->tableWidget->setCurrentItem(ui->tableWidget->item(0,0));
}

void GeneralOverview::resetButtons(){
    QLayoutItem *child;
    while((child = button_layout->takeAt(0))){
        QWidget *widget = child->widget();
        if(widget)
            delete widget;
    }
}

GeneralOverview::~GeneralOverview()
{
    delete ui;
}

void GeneralOverview::addLines(const Skill *skill){
    QString skill_name = Sanguosha->translate(skill->objectName());
    QStringList sources = skill->getSources();

    if(sources.isEmpty()){
        QCommandLinkButton *button = new QCommandLinkButton(skill_name);

        button->setEnabled(false);
        button_layout->addWidget(button);
    }else{
        QRegExp rx(".+/(\\w+\\d?).ogg");
        int i;
        for(i=0; i<sources.length(); i++){
            QString source = sources.at(i);
            if(!rx.exactMatch(source))
                continue;

            QString button_text = skill_name;
            if(sources.length() != 1){
                button_text.append(QString(" (%1)").arg(i+1));
            }

            QCommandLinkButton *button = new QCommandLinkButton(button_text);
            button->setObjectName(source);
            button_layout->addWidget(button);

            QString filename = rx.capturedTexts().at(1);
            QString skill_line = Sanguosha->translate("$" + filename);
            button->setDescription(skill_line);

            connect(button, SIGNAL(clicked()), this, SLOT(playEffect()));

            addCopyAction(button);
        }
    }
}

void GeneralOverview::addCopyAction(QCommandLinkButton *button){
    QAction *action = new QAction(button);
    action->setData(button->description());
    button->addAction(action);
    action->setText(tr("Copy lines"));
    button->setContextMenuPolicy(Qt::ActionsContextMenu);

    connect(action, SIGNAL(triggered()), this, SLOT(copyLines()));
}

void GeneralOverview::copyLines(){
    QAction *action = qobject_cast<QAction *>(sender());
    if(action){
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(action->data().toString());
    }
}

void GeneralOverview::on_tableWidget_itemSelectionChanged()
{
    int row = ui->tableWidget->currentRow();
    QString general_name = ui->tableWidget->item(row, 0)->data(Qt::UserRole).toString();
    const General *general = Sanguosha->getGeneral(general_name);
    ui->generalPhoto->setPixmap(QPixmap(general->getPixmapPath("card")));
    QList<const Skill *> skills = general->getVisibleSkillList();
    ui->skillTextEdit->clear();

    resetButtons();

    foreach(const Skill *skill, skills){
        addLines(skill);
    }

    QString last_word = Sanguosha->translate("~" + general->objectName());
    if(!last_word.startsWith("~")){
        QCommandLinkButton *death_button = new QCommandLinkButton(tr("Death"), last_word);
        button_layout->addWidget(death_button);

        connect(death_button, SIGNAL(clicked()), general, SLOT(lastWord()));

        addCopyAction(death_button);
    }

    if(general_name == "caocao" || general_name == "shencc" || general_name == "shencaocao"){
        QCommandLinkButton *win_button = new QCommandLinkButton(tr("Victory"), tr(
                "Six dragons lead my chariot, "
                "I will ride the wind with the greatest speed."
                "With all of the feudal lords under my command,"
                "to rule the world with one name!"));

        button_layout->addWidget(win_button);
        addCopyAction(win_button);

        win_button->setObjectName("audio/system/win-cc.ogg");
        connect(win_button, SIGNAL(clicked()), this, SLOT(playEffect()));
    }

    QString designer_text = Sanguosha->translate("designer:" + general->objectName());
    if(!designer_text.startsWith("designer:"))
        ui->designerLineEdit->setText(designer_text);
    else
        ui->designerLineEdit->setText(tr("Official"));

    QString cv_text = Sanguosha->translate("cv:" + general->objectName());
    if(!cv_text.startsWith("cv:"))
        ui->cvLineEdit->setText(cv_text);
    else
        ui->cvLineEdit->setText(tr("Official"));

    button_layout->addStretch();
    ui->skillTextEdit->append(general->getSkillDescription());
}

void GeneralOverview::playEffect()
{
    QObject *button = sender();
    if(button){
        QString source = button->objectName();
        if(!source.isEmpty())
            Sanguosha->playEffect(source);
    }
}
