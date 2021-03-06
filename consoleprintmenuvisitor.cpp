#include "consoleprintmenuvisitor.h"
#include "menuitem.h"
#include "menu.h"

#include <iostream>

/*!
 * \brief ConsolePrintMenuVisitor::indent
 * \param item
 * \return
 */
std::string ConsolePrintMenuVisitor::indent(AbstractMenuItem  *item) const
{
    std::string rIndentString;
    return rIndentString;
}

/*!
 * \brief ConsolePrintMenuVisitor::visit
 * \param item
 */
void ConsolePrintMenuVisitor::visit(MenuItem *item)
{
    std::string lIndentString = indent(item);

//    if(!item->description().empty())
//    std::cout << lIndentString <<"*** NEW  ";

    std::cout << lIndentString <<"NEW"<< "> " << item->title() << "    :        " << item->price() << "$" << std::endl;

    if (!item->description().empty())
    {
        std::cout << lIndentString << "    ::::" << item->description() << "::::" << std::endl;
    }
}

/*!
 * \brief ConsolePrintMenuVisitor::visit
 * \param menu
 */
void ConsolePrintMenuVisitor::visit(Menu *menu)
{
    std::cout << indent(menu) << "[" << menu->title() << "]" << std::endl;
}
