# Zad_2
Uruchamiają wszystkie testy dla statycznej/współdzielonej bibliotek:   
`make shared_tests`   
`make static_tests`   
\
Wyniki testów znadjdują się w pliku zad2_report.txt
# Zad_3a
Uruchamiają wszystkie testy dla statycznej/współdzielonej/dll bibliotek:   
`make shared_tests`   
`make static_tests`   
`make dynamic_tests`   
\
Wyniki znajdują się w odpowiednich wykonanemu poleceniu plikach
# Zad_3b
Uruchomić testy z różnymi poziomami optymalizacji:   
`make {shared_tests|static_tests|dynamic_tests} Opt={1|2|3|0|s| }`   
Gdzie {a|b} to a lub b   
Wyniki znajdują się w odpowiednich wykonanemu poleceniu plikach   
___
# Wnioski
W większości przypadków nieco szybciej wykonywały się testy, korzystające ze statycznej biblioteki (choć różnica i nie była duża).   
Jednak, mierząc czas wykonania polecenia, a nie całego programu można zauważyć, że testy, korzystające z dll wykonują się szybciej.   
Odnośnie poszczególnych operacji, to najdłuższą zdecydowanie jest merge_files i write_to_tmp_file, w związku z korzystaniem z I/O system cpu jest największy dla tych operacji.   
Realtime jest mniej więcej równy sumie System cpu + User cpu, co się zgadza z oczekiwaniami.   
Różne poziomy kompilacji dają przyśpieszenie, równe 5-7% w zależności od wybranego poziomu optymalizacji. W moim przypadku najbardziej się sprawiło -O2, co zwiększyło i czas kompilacji, ale dla takich małych programów to zwiększenie jest ledwie zauważalne.   

## Uwagi
Przykładowy czas wykonania wszystkich testów ~20 sec
