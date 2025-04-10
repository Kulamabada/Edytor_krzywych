import os
import argparse
import datetime


def collect_files(project_dir="C:\\Users\\julek\\CLionProjects\\edytor krzywych",
                  output_dir="C:\\Users\\julek\\CLionProjects\\edytor krzywych\\temp",
                  output_file="temp.txt",
                  extensions=[".cpp", ".h", ".hpp"]):
    """
    Zbiera wszystkie pliki projektu C++ z podanych rozszerzeń do jednego pliku tekstowego.
    """
    # Upewnij się, że katalog wyjściowy istnieje
    os.makedirs(output_dir, exist_ok=True)

    # Pełna ścieżka do pliku wyjściowego
    output_path = os.path.join(output_dir, output_file)

    excluded_dirs = ['cmake-build-debug', 'cmake-build-release', '.idea', 'CMakeFiles', 'eigen-3.4.0']
    collected_files = []

    print(f"Przeszukuję katalog: {project_dir}")
    print(f"Szukam plików z rozszerzeniami: {extensions}")

    # Przejdź przez wszystkie pliki i podkatalogi
    for root, dirs, files in os.walk(project_dir):
        dirs[:] = [d for d in dirs if d not in excluded_dirs]
        for file in files:
            if any(file.endswith(ext) for ext in extensions):
                file_path = os.path.join(root, file)
                collected_files.append(file_path)

    # Sortuj pliki dla lepszej czytelności
    collected_files.sort()

    if not collected_files:
        print(f"Nie znaleziono plików z rozszerzeniami {extensions} w katalogu {project_dir}")
        return []

    # Zapisz zawartość wszystkich plików do pliku wyjściowego
    with open(output_path, 'w', encoding='utf-8') as out_file:
        # Dodaj nagłówek z datą i informacją o pliku
        timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        out_file.write(f"// Plik wygenerowany automatycznie {timestamp}\n")
        out_file.write(f"// Zawiera {len(collected_files)} plików z projektu edytora krzywych\n\n")

        for file_path in collected_files:
            rel_path = os.path.relpath(file_path, project_dir)
            out_file.write(f"//===== START FILE: {rel_path} =====//\n\n")

            try:
                with open(file_path, 'r', encoding='utf-8') as in_file:
                    content = in_file.read()
                    out_file.write(content)
                    # Dodaj nową linię jeśli plik nie kończy się nową linią
                    if content and not content.endswith('\n'):
                        out_file.write('\n')
            except UnicodeDecodeError:
                # Spróbuj z innym kodowaniem jeśli UTF-8 zawiedzie
                with open(file_path, 'r', encoding='latin-1') as in_file:
                    content = in_file.read()
                    out_file.write(content)
                    if content and not content.endswith('\n'):
                        out_file.write('\n')
            except Exception as e:
                out_file.write(f"// BŁĄD ODCZYTU PLIKU: {str(e)}\n")

            out_file.write(f"\n//===== END FILE: {rel_path} =====//\n\n")

    print(f"Pomyślnie zapisano {len(collected_files)} plików do {output_path}")

    # Wyświetl listę skopiowanych plików
    print("\nSkopiowane pliki:")
    for i, file_path in enumerate(collected_files):
        print(f"{i + 1}. {os.path.relpath(file_path, project_dir)}")

    return collected_files


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Zbiera pliki projektu C++/Qt do jednego pliku tekstowego")
    parser.add_argument("--dir",
                        help="Katalog startowy projektu (domyślnie: C:\\Users\\julek\\CLionProjects\\edytor krzywych)")
    parser.add_argument("--out", help="Nazwa pliku wyjściowego (domyślnie: temp.txt)")
    parser.add_argument("--out-dir",
                        help="Katalog wyjściowy (domyślnie: C:\\Users\\julek\\CLionProjects\\edytor krzywych\\temp)")
    parser.add_argument("--exts", help="Rozszerzenia plików do zebrania (domyślnie: .cpp,.h,.hpp)")

    args = parser.parse_args()

    project_dir = args.dir or "C:\\Users\\julek\\CLionProjects\\edytor krzywych"
    output_dir = args.out_dir or "C:\\Users\\julek\\CLionProjects\\edytor krzywych\\temp"
    output_file = args.out or "temp.txt"
    extensions = [ext.strip() for ext in (args.exts or ".cpp,.h,.hpp").split(",")]

    collect_files(project_dir, output_dir, output_file, extensions)
    os.system("pause")