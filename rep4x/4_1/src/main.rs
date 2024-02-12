use std::sync::{Arc, Mutex};
use std::thread;
use std::time::{Duration, Instant};
use std::fs::File;
use std::io::Write;

fn is_prime(number: i32) -> bool {
    if number <= 1 {
        return false;
    }
    if number == 2 {
        return true;
    }
    if number % 2 == 0 {
        return false;
    }

    let mut i = 3;
    while i * i <= number {
        if number % i == 0 {
            return false;
        }
        i += 2;
    }

    true
}

fn worker(jobs: Arc<Mutex<Vec<i32>>>, results: Arc<Mutex<Vec<(i32, usize)>>>, worker_id: usize, worker_count: usize) {
    let mut jobs = jobs.lock().unwrap();
    let mut results = results.lock().unwrap();
    while let Some(number) = jobs.pop() {
        if number % worker_count as i32 == worker_id as i32 && is_prime(number) {
            results.push((number, worker_id));
        }
    }
}

fn calculate_primes_concurrent(worker_count: usize, start: i32, end: i32, jobs: Arc<Mutex<Vec<i32>>>, results: Arc<Mutex<Vec<(i32, usize)>>>) -> (Vec<(i32, usize)>, Duration) {
    let mut handles = vec![];
    let start_time = Instant::now();

    for i in 0..worker_count {
        let jobs = Arc::clone(&jobs);
        let results = Arc::clone(&results);
        let handle = thread::spawn(move || {
            worker(jobs, results, i, worker_count);
        });
        handles.push(handle);
    }

    for handle in handles {
        handle.join().unwrap();
    }

    let results = Arc::try_unwrap(results).unwrap().into_inner().unwrap();
    (results, start_time.elapsed())
}


fn calculate_primes_single_thread(start: i32, end: i32) -> (Vec<i32>, Duration) {
    let start_time = Instant::now();
    let primes: Vec<i32> = (start..=end).into_iter().filter(|&x| is_prime(x)).collect();
    (primes, start_time.elapsed())
}

fn generate_svg(worker_counts: &[usize], times: &[f64]) -> String {
    let max_time = *times.iter().max_by(|x, y| x.partial_cmp(y).unwrap()).unwrap_or(&1.0);

    let width = 400;
    let height = 300;

    let mut svg_content = String::new();
    svg_content.push_str(&format!(r#"<svg width="{}" height="{}" xmlns="http://www.w3.org/2000/svg">"#, width, height));

    // Draw Y-axis labels
    for i in 0..=10 {
        let y = (height as f64 - (i as f64 / 10.0) * height as f64) as i32;
        let label = format!("{:.2}s", i as f64 * max_time / 10.0);
        svg_content.push_str(&format!(
            r#"<text x="20" y="{}" font-size="10" fill="black">{}</text>"#,
            y, label
        ));
    }

    // Draw X-axis labels and lines
    for (i, &worker_count) in worker_counts.iter().enumerate() {
        let x = ((i as f64 / worker_counts.len() as f64) * (width as f64 - 50.0) + 50.0) as i32;
        let y = (height as f64 - (times[i] / max_time) * height as f64) as i32;
        let label = format!("{}", worker_count);

        // Draw X-axis label
        svg_content.push_str(&format!(
            r#"<text x="{}" y="{}" font-size="12" fill="black" text-anchor="middle">{}</text>"#,
            x, height - 5, label
        ));

        // Draw line to connect points
        if i > 0 {
            let prev_x = (((i - 1) as f64 / worker_counts.len() as f64) * (width as f64 - 50.0) + 50.0) as i32;
            let prev_y = (height as f64 - (times[i - 1] / max_time) * height as f64) as i32;

            svg_content.push_str(&format!(
                r#"<line x1="{}" y1="{}" x2="{}" y2="{}" stroke="black" />"#,
                prev_x, prev_y, x, y
            ));
        }
    }

    svg_content.push_str("</svg>");
    svg_content
}

fn main() {
    let m = 10000000;
    let n = 20000000;

    let worker_counts = vec![1, 2, 3, 4];
    let mut handles = vec![];
    let times = Arc::new(Mutex::new(vec![]));

    for &worker_count in &worker_counts {
        let jobs = Arc::new(Mutex::new((3..=m).collect::<Vec<_>>()));
        let results = Arc::new(Mutex::new(Vec::new()));
        let times = Arc::clone(&times);

        let handle = thread::spawn(move || {
            let (primes_parallel, elapsed_parallel) =
                calculate_primes_concurrent(worker_count, 3, m, jobs, results);
            let (primes_single, elapsed_single) = calculate_primes_single_thread(m + 1, n);
            let total_elapsed = elapsed_parallel + elapsed_single;

            let mut times = times.lock().unwrap();
            times.push(total_elapsed);

            println!("Total time with {} worker(s): {:?}", worker_count, total_elapsed);
        });

        handles.push(handle);
    }

    for handle in handles {
        handle.join().unwrap();
    }

    let times: Vec<f64> = times.lock().unwrap().iter().map(|&duration| duration.as_secs_f64()).collect();

    // generate_svg 関数を呼び出し、その結果を変数に保存する
    let svg_content = generate_svg(&worker_counts, &times);

    // SVG コンテンツをファイルに書き込む
    let mut file = File::create("output.svg").expect("Unable to create file");
    file.write_all(svg_content.as_bytes()).expect("Unable to write to file");

    // SVG コンテンツを標準出力にも出力
    println!("{}", svg_content);
}
