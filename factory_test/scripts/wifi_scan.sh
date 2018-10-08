$1 == "BSS" {
    MAC = $2
    wifi[MAC]["BSSID"] = substr($2,1,17)
}
$1 == "SSID:" {
    wifi[MAC]["SSID"] = $2
}
$1 == "freq:" {
    wifi[MAC]["freq"] = $NF
}
$1 == "signal:" {
    wifi[MAC]["sig"] = $2 " " $3
}
END {
    printf "%s\t\t\t\t%s\t%s\t\t%s\n", "BSSID", "Frequency", "Signal", "SSID"

    for (w in wifi) {
        printf "%s\t\t%s\t\t%s\t\t%s\n", wifi[w]["BSSID"], wifi[w]["freq"],wifi[w]["sig"],wifi[w]["SSID"]
    }
}
