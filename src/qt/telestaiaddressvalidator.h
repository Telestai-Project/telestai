// Copyright (c) 2011-2014 The Bitcoin Core developers
// Copyright (c) 2017-2019 The Telestai Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TELESTAI_QT_TELESTAIADDRESSVALIDATOR_H
#define TELESTAI_QT_TELESTAIADDRESSVALIDATOR_H

#include <QValidator>

/** Base58 entry widget validator, checks for valid characters and
 * removes some whitespace.
 */
class TelestaiAddressEntryValidator : public QValidator
{
    Q_OBJECT

public:
    explicit TelestaiAddressEntryValidator(QObject *parent);

    State validate(QString &input, int &pos) const;
};

/** Telestai address widget validator, checks for a valid telestai address.
 */
class TelestaiAddressCheckValidator : public QValidator
{
    Q_OBJECT

public:
    explicit TelestaiAddressCheckValidator(QObject *parent);

    State validate(QString &input, int &pos) const;
};

#endif // TELESTAI_QT_TELESTAIADDRESSVALIDATOR_H
