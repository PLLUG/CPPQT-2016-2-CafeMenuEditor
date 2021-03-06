#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <utility>

#include <menu.h>
#include <menuitem.h>
#include <QJsonObject>
#include <string>
#include <queue>
#include <QJsonArray>
#include <QFileDialog>
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QDir>
#include <map>
#include <tuple>

#include "texteditprintmenuvisitor.h"
#include "menuiterator.h"
#include "adddialog.h"
#include "ui_adddialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    resize(800, 600);

    createMenu();
    slotUpdateMenu();

    connect(ui->menuComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(menuElementSelected()), Qt::UniqueConnection);
    connect(ui->action_Add, SIGNAL(triggered(bool)),
            this, SLOT(slotAddNewItem()), Qt::UniqueConnection);
    connect(ui->savePushButton, SIGNAL(clicked(bool)),
            this, SLOT(slotSaveEditedItem()), Qt::UniqueConnection);
    connect(ui->menuEditorDelegate, SIGNAL(itemChanged()),
            this, SLOT(slotItemChanged()), Qt::UniqueConnection);
    //First task. Add new button '+'
    connect(ui->addButton, SIGNAL(clicked(bool)),
            this, SLOT(slotAddNewItem()), Qt::UniqueConnection);
    connect(ui->action_Open, SIGNAL(triggered(bool)),
             this, SLOT(slotOpenFile()), Qt::UniqueConnection);
    connect(ui->buttonDelete, SIGNAL(clicked(bool)), this, SLOT(slotDeleteItem()), Qt::UniqueConnection);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slotPrintMenu()
{
    ui->menuTextEdit->clear();

    TextEditPrintMenuVisitor visitor(ui->menuTextEdit);

    MenuIterator iterator(mRoot.get());
    while(iterator.hasNext())
    {
        auto item =  iterator.next();
        item->apply(&visitor);
    }
}

void MainWindow::menuElementSelected()
{
    ui->savePushButton->setEnabled(false);

    AbstractMenuItem * item = ui->menuComboBox->currentMenuItem();

    AbstractVisitor *visitor = ui->menuEditorDelegate;
    item->apply(visitor);
    ui->buttonDelete->setEnabled(true);
}

void MainWindow::slotUpdateMenu()
{
    int index = ui->menuComboBox->currentIndex();

    ui->menuComboBox->setMenu(mRoot.get());

    slotPrintMenu();

    ui->menuComboBox->setCurrentIndex(index);
}

void MainWindow::slotAddNewItem()
{
    AddDialog addDialog;
    addDialog.setMenu(mRoot.get());


    if (QDialog::Accepted == addDialog.exec())
    {

        slotUpdateMenu();
    }
}

void MainWindow::slotItemChanged()
{
    ui->savePushButton->setEnabled(true);
}

void MainWindow::slotSaveEditedItem()
{
    if(ui->menuEditorDelegate->slotSave())
    {
            if(ui->menuComboBox->checkDublicate(ui->menuEditorDelegate->getTempTitle()))
            {
             ui->savePushButton->setEnabled(false);
             slotUpdateMenu();
            }
            else
            {
                QMessageBox msgBox;
                msgBox.setText("Menu item with the same name already exists.");
                msgBox.exec();
            }
    }
}

void MainWindow::slotOpenFile()
{
    QFileDialog openDialog(this, tr("Open File..."), QDir::currentPath(), tr("Json Files (*.json);;All Files (*.*)"));
    openDialog.setAcceptMode(QFileDialog::AcceptOpen);
    if(QDialog::Accepted == openDialog.exec())
    {
        auto filePath = openDialog.selectedFiles()[0];
        QFile file(filePath);
        file.open(QIODevice::ReadOnly);
        QTextStream file_text(&file);
        QString json_string;
        json_string = file_text.readAll();
        file.close();
        QByteArray json_bytes = json_string.toLocal8Bit();
        QJsonParseError parseErrorJson;
        auto result = QJsonDocument::fromJson(json_bytes, &parseErrorJson);
        if(parseErrorJson.error == QJsonParseError::NoError)
        {
            auto arrJson = result.object();
            mRoot.reset();
            auto root = std::make_unique<Menu>("MAIN MENU");
            root->append(std::move(createMenuFromJson(arrJson)));
            mRoot = std::move(root);
            slotUpdateMenu();
        }
        else
        {
            QMessageBox info(QMessageBox::Warning, tr("Error"), QString("Error in reading json file: %1.").arg(parseErrorJson.errorString()));
            info.exec();
        }
    }
}

void MainWindow::slotDeleteItem()
{
    auto toDel = ui->menuComboBox->currentMenuItem();
    ui->menuComboBox->clearData();
    toDel->removeSubitem();
    ui->menuComboBox->updateComboBox();
    slotPrintMenu();
    ui->menuComboBox->updateComboBox();
    if(ui->menuComboBox->currentIndex() < 0)
    {
        ui->buttonDelete->setEnabled(false);
    }

}

void MainWindow::createMenu()
{
    auto root = std::make_unique<Menu>("MAIN MENU");

    auto pizzaMenu = std::make_unique<Menu>("Pizza Menu");
    pizzaMenu->append(std::make_unique<MenuItem>("hawaiian pizza", 2.4, "cheese and tomato base with toppings of ham and pineapple"));
    pizzaMenu->append(std::make_unique<MenuItem>("vegetarian pizza", 4.2, "cheese and tomato ... "));


    auto beveragesMenu = std::make_unique<Menu>("Beverages");
    beveragesMenu->append(std::make_unique<MenuItem>("Coca-Cola", 2));

    auto coffeMenu = std::make_unique<Menu>("Coffe");
    coffeMenu->append(std::make_unique<MenuItem>("Late", 1, "             "));
    coffeMenu->append(std::make_unique<MenuItem>("Capucino", 2, "             "));
    beveragesMenu->append(std::move(coffeMenu));

    beveragesMenu->append(std::make_unique<MenuItem>("Pepsi-Cola", 3));

    auto mineralWatersMenu = std::make_unique<Menu>("Mineral waters");
    mineralWatersMenu->append(std::make_unique<MenuItem>("Borjomi", 2.43, "   nice thing  "));
    mineralWatersMenu->append(std::make_unique<MenuItem>("Morshynska", 1.4, "         "));
    beveragesMenu->append(std::move(mineralWatersMenu));

    auto alcoDrinksMenu = std::make_unique<Menu>("Alco drinks");
    auto winesMenu = std::make_unique<Menu>("Wines");
    auto dryWines = std::make_unique<Menu>("Dry Wines");
    dryWines->append(std::make_unique<MenuItem>("Bordeaux", 20));
    winesMenu->append(std::move(dryWines));
    winesMenu->append(std::make_unique<MenuItem>("Champagne", 16.5));
    alcoDrinksMenu->append(std::move(winesMenu));
    alcoDrinksMenu->append(std::make_unique<MenuItem>("Beer", 5));
    beveragesMenu->append(std::move(alcoDrinksMenu));
    root->append(std::move(beveragesMenu));
    root->append(std::move(pizzaMenu));
    mRoot = std::move(root);
}

std::unique_ptr<AbstractMenuItem> MainWindow::createMenuFromJson(QJsonObject sub)
{
     auto keys = sub.keys();
     int index = 0;
     std::queue<QJsonArray> children;
     std::map<std::string, std::string> valueStr;
     std::map<std::string, double> valueDouble;
     std::map<std::string, bool> valueBool;
     enum typeData{Str, Dbl, Bool};
     auto mixedData = make_tuple(valueStr, valueDouble, valueBool); //tuple of 3 kind of data value:str <-> key:{str, dbl, bool}.
     for(auto i : sub)
     {
         if(i.isArray())
         {
             //foo(i.toArray(), deep+1);
             children.push(i.toArray());
             ++index;
         }
         else
         {
             QString key = keys[index++];
             if(i.isString())
             {
                 std::get<typeData::Str>(mixedData)[key.toStdString()] = i.toString().toStdString();
             }
             if(i.isDouble())
             {
                 std::get<typeData::Dbl>(mixedData)[key.toStdString()] = i.toDouble();
             }
             if(i.isBool())
             {
                 std::get<typeData::Bool>(mixedData)[key.toStdString()] = i.toBool();
             }
         }
     }
     //create new menu or menuitem
     std::unique_ptr<MenuItem> newMenuItem;
     std::unique_ptr<Menu> newMenu;
     if(std::get<typeData::Str>(mixedData)["type"] == std::string("Menu"))
     {
         newMenu = std::make_unique<Menu>(std::get<typeData::Str>(mixedData)["title"]/*valueStr["title"]*/);
         while(!children.empty())
         {
             //append all children
             auto currentItem = children.front();
             children.pop();
             for(auto j : currentItem)
             {
                 newMenu->append(createMenuFromJson(j.toObject()));
             }
         }
         return std::move(newMenu);
     }
     else
     {
         newMenuItem = std::make_unique<MenuItem>(std::get<typeData::Str>(mixedData)["title"],
                 std::get<typeData::Dbl>(mixedData)["price"],
                 std::get<typeData::Str>(mixedData)["description"]);
         return std::move(newMenuItem);
     }
}
