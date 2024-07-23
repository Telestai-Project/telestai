// Copyright (c) 2017-2019 The Telestai Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TELESTAICOIN_ASSETFILTERPROXY_H
#define TELESTAICOIN_ASSETFILTERPROXY_H

#include <QSortFilterProxyModel>

class AssetFilterProxy : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit AssetFilterProxy(QObject *parent = 0);

    void setAssetNamePrefix(const QString &assetNamePrefix);
    void setAssetNameContains(const QString &assetNameContains);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex & source_parent) const;

private:
    QString assetNamePrefix;
    QString assetNameContains;
};


#endif //TELESTAICOIN_ASSETFILTERPROXY_H
