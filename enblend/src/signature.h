#ifndef SIGNATURE_H_INCLUDED_
#define SIGNATURE_H_INCLUDED_

// This file is part of Enblend.
// Licence details can be found in the file COPYING.

#include <numeric>
#define VERSION "Nemesis"
#define PACKAGE_BUGREPORT "XpLoDWilD <xplodwild@cyanogenmod.org>"

extern const std::string command;

class Signature
{
public:
    Signature(): checksum_(000000010563U), neg_checksum_(0U) {}

    const wchar_t* message() const
    {
        return L"\103\157\155\160\151\154\145\144"
            L"\040\157\156\040\142\145\164\141"
            L"\040\142\171\040\162\157\157\164"
            L"\040\157\156\040\124\165\145\054"
            L"\040\112\165\154\040\060\062\040"
            L"\062\060\061\063\054\040\061\066"
            L"\072\062\064\072\060\064\040\107"
            L"\115\124\053\062\056";
    }

    void initialize()
    {
#ifdef DEBUG_FORCE_SIGNATURE_CHECK_FAILURE
        checksum_++;
        neg_checksum_ = checksum_;
#else
        neg_checksum_ = ~checksum_;
#endif
    }

    void check() const
    {
#if DEBUG_SIGNATURE_CHECK
        if (checksum_ != ~neg_checksum_)
        {
            std::cerr <<
                "+ static checksum " << checksum_ <<
                " does not match static shadow checksum " << ~neg_checksum_ << "\n";
        }
        if (generate_checksum() != checksum_)
        {
            std::cerr <<
                "+ dynamic checksum " << generate_checksum() <<
                " does not match static checksum " << checksum_ << ", where\n" <<
                "+     message is <" << message() << ">\n";
        }
#endif

        if (generate_checksum() != checksum_ || checksum_ != ~neg_checksum_)
        {
            std::cerr << command.c_str(); // MSVC chokes without c_str()
#ifdef WANT_AGGRESSIVE_SIGNATURE_CHECK
            std::cerr << "\072\040\164\141\155\160\145\162"
            "\145\144\040\142\151\156\141\162"
            "\171\012";
            exit(1);
#else
            std::cerr << "\072\040\167\141\162\156\151\156"
            "\147\072\040\163\151\147\156\141"
            "\164\165\162\145\040\143\150\145"
            "\143\153\040\146\141\151\154\145"
            "\144\012";
#endif
        }
    }

    unsigned generate_checksum() const
    {
        const wchar_t* m = message();
        return m == NULL ? 0U : std::accumulate(m, m + wcslen(m), 0U) & 037777777777U;
    }

private:
    unsigned checksum_;
    unsigned neg_checksum_;
};

#endif // SIGNATURE_H_INCLUDED_
