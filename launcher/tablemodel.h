#include <QAbstractTableModel>
#include <Qt>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlTableModel>

namespace Tetradactyl {

class ApplicationTableModel : public QSqlTableModel {
  Q_OBJECT
public:
  ApplicationTableModel();
  virtual ~ApplicationTableModel();

private:
  QSqlDatabase *db;
};

} // namespace Tetradactyl
